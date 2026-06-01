#!/usr/bin/env python3
import argparse
import json
import os
import socket
import sys
import time

MULTICAST_GROUP = '239.255.255.250'
PORT = 1900

KNOWN_TYPES = [
    'WRISTBAND', 'FIRE_SENSOR', 'PIR_SENSOR', 'DOOR_LOCK',
    'SPRINKLER', 'LIGHT', 'SPEAKER', 'DIALER'
]

KNOWN_ROOMS = ['KITCHEN', 'LIVING_ROOM', 'BEDROOM', 'BATHROOM', 'HALLWAY']


def normalize_nt(value: str) -> str:
    value = value.lower()
    if value == 'alive':
        return 'ssdp:alive'
    if value == 'byebye':
        return 'ssdp:byebye'
    if value in ('ssdp:alive', 'ssdp:byebye'):
        return value
    raise ValueError('Invalid NTS value: ' + value)


def build_message(nt, device_id, dev_type, room, config_group=MULTICAST_GROUP, port=PORT, ttl=2):
    msg = []
    msg.append('NOTIFY * HTTP/1.1\r\n')
    msg.append(f'HOST: {config_group}:{port}\r\n')
    msg.append('NT: urn:smarthome:device:1\r\n')
    msg.append(f'NTS: {nt}\r\n')
    msg.append(f'USN: uuid:{device_id}::type:{dev_type}::room:{room}\r\n')
    if nt == 'ssdp:alive':
        msg.append(f'CACHE-CONTROL: max-age={ttl}\r\n')
        msg.append(f'LOCATION: mqtt://{device_id}\r\n')
    msg.append('\r\n')
    return ''.join(msg)


def send(msg, group=MULTICAST_GROUP, port=PORT, ttl=2):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)
    sock.sendto(msg.encode('utf-8'), (group, port))
    sock.close()


def parse_device_spec(spec):
    parts = spec.split(':')
    if len(parts) != 4:
        raise argparse.ArgumentTypeError(
            'Device spec must be DEVICE_ID:TYPE:ROOM:NTS'
        )
    device_id, dev_type, room, nts = parts
    nts = normalize_nt(nts)
    if dev_type not in KNOWN_TYPES:
        raise argparse.ArgumentTypeError(f'Unknown device type: {dev_type}')
    if room not in KNOWN_ROOMS:
        raise argparse.ArgumentTypeError(f'Unknown room: {room}')
    return device_id, dev_type, room, nts


def load_json_scenario(path):
    if not os.path.isfile(path):
        raise FileNotFoundError(path)
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    messages = []
    for entry in data:
        if not all(k in entry for k in ('id', 'type', 'room', 'nt')):
            raise ValueError('JSON scenario entries must contain id,type,room,nt')
        messages.append(parse_device_spec(f"{entry['id']}:{entry['type']}:{entry['room']}:{entry['nt']}") )
    return messages


def scenario_devices(name):
    if name == 'three-lists':
        return [
            ('FIRE_KITCHEN', 'FIRE_SENSOR', 'KITCHEN', 'ssdp:alive'),
            ('FIRE_KITCHEN', 'FIRE_SENSOR', 'KITCHEN', 'ssdp:byebye'),
            ('UNKNOWN_DEVICE', 'PIR_SENSOR', 'HALLWAY', 'ssdp:alive')
        ]
    if name == 'active-only':
        return [
            ('FIRE_KITCHEN', 'FIRE_SENSOR', 'KITCHEN', 'ssdp:alive')
        ]
    if name == 'excluded-only':
        return [
            ('UNKNOWN_DEVICE', 'PIR_SENSOR', 'HALLWAY', 'ssdp:alive')
        ]
    if name == 'unavailable-only':
        return [
            ('FIRE_KITCHEN', 'FIRE_SENSOR', 'KITCHEN', 'ssdp:byebye')
        ]
    raise ValueError('Unknown scenario: ' + name)


def read_choice(prompt, allowed):
    while True:
        choice = input(prompt).strip()
        if choice in allowed:
            return choice
        print('Please choose one of: ' + ', '.join(allowed))


def prompt_custom_device():
    device_id = input('Device ID (example: FIRE_KITCHEN): ').strip()
    dev_type = input(f'Device Type {KNOWN_TYPES}: ').strip()
    room = input(f'Room {KNOWN_ROOMS}: ').strip()
    nts = normalize_nt(input('NTS (alive/byebye): ').strip())
    return device_id, dev_type, room, nts


def prompt_menu():
    print('Choose a test flow:')
    print(' 1) three-lists (active, unavailable, excluded)')
    print(' 2) active-only')
    print(' 3) excluded-only')
    print(' 4) unavailable-only')
    print(' 5) custom device')
    print(' 6) JSON scenario file')
    print(' 0) exit')
    choice = read_choice('Select option: ', [str(i) for i in range(7)])
    if choice == '0':
        sys.exit(0)
    if choice == '1':
        return scenario_devices('three-lists')
    if choice == '2':
        return scenario_devices('active-only')
    if choice == '3':
        return scenario_devices('excluded-only')
    if choice == '4':
        return scenario_devices('unavailable-only')
    if choice == '5':
        return [prompt_custom_device()]
    if choice == '6':
        path = input('JSON scenario file path: ').strip()
        return load_json_scenario(path)
    return []


def main():
    parser = argparse.ArgumentParser(description='Send SSDP NOTIFY test messages')
    parser.add_argument('--device', '-d', action='append', type=parse_device_spec,
                        help='Device spec: DEVICE_ID:TYPE:ROOM:NTS')
    parser.add_argument('--scenario', '-s', help='Built-in scenario name or JSON file path')
    parser.add_argument('--repeat', '-r', type=int, default=1,
                        help='Number of times to send each message')
    parser.add_argument('--interval', '-i', type=float, default=1.0,
                        help='Seconds to wait between repeats')
    parser.add_argument('--ttl', type=int, default=2,
                        help='IP multicast TTL')
    parser.add_argument('--group', default=MULTICAST_GROUP,
                        help='Multicast group to send to')
    parser.add_argument('--port', type=int, default=PORT,
                        help='Destination UDP port')
    args = parser.parse_args()

    messages = []
    if args.device:
        messages.extend(args.device)
    if args.scenario:
        if args.scenario.endswith('.json'):
            messages.extend(load_json_scenario(args.scenario))
        else:
            messages.extend(scenario_devices(args.scenario))

    if not messages:
        messages = prompt_menu()

    print(f'Sending {len(messages)} messages, repeat={args.repeat}, interval={args.interval}s')
    for attempt in range(args.repeat):
        for device_id, dev_type, room, nts in messages:
            msg = build_message(nts, device_id, dev_type, room, args.group, args.port, args.ttl)
            print(f'[{attempt+1}/{args.repeat}] {device_id} {dev_type} {room} {nts}')
            print(msg)
            send(msg, args.group, args.port, args.ttl)
        if attempt + 1 < args.repeat:
            time.sleep(args.interval)
    print('Done')


if __name__ == '__main__':
    main()
