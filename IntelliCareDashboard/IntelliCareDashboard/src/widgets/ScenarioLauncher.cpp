#include "ScenarioLauncher.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QApplication>
#include <QStandardPaths>
#include <QFont>
#include <QScrollBar>

ScenarioLauncher::ScenarioLauncher(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Scenario Launcher — IntelliCare");
    setMinimumSize(780, 560);
    setModal(false);  // Non-modal — dashboard ostaje vidljiv

    // ── Pokušaj auto-detect putanje ────────────────────────────────────────
    // Traži intellicare/ folder relativno od exe
    QDir appDir(QApplication::applicationDirPath());
    QStringList candidates = {
        appDir.absolutePath() + "/../intellicare",
        appDir.absolutePath() + "/../../intellicare",
        appDir.absolutePath() + "/../../../intellicare",
        QDir::homePath() + "/intellicare",
    };
    for (const auto& c : candidates) {
        if (QDir(c).exists()) {
            m_scenarioDir   = QDir(c + "/scenarios").absolutePath();
            m_testHousePath = QDir(c).absolutePath() + "/test_house";
            break;
        }
    }

    // ── Layout ────────────────────────────────────────────────────────────
    auto* mainVL = new QVBoxLayout(this);
    mainVL->setSpacing(10);
    mainVL->setContentsMargins(14, 14, 14, 14);

    // Title bar
    auto* titleRow = new QHBoxLayout;
    auto* title = new QLabel("🎬  Scenario Launcher");
    title->setStyleSheet("font-size:16px; font-weight:700; color:#1f2937;");
    titleRow->addWidget(title);
    titleRow->addStretch();
    mainVL->addLayout(titleRow);

    // Path row
    auto* pathRow = new QHBoxLayout;
    m_pathLabel = new QLabel(m_testHousePath.isEmpty()
        ? "⚠  test_house not found — please browse manually"
        : "📁  " + m_testHousePath);
    m_pathLabel->setStyleSheet(m_testHousePath.isEmpty()
        ? "font-size:11px; color:#f59e0b;"
        : "font-size:11px; color:#6b7280;");
    m_pathLabel->setWordWrap(true);

    m_browseBtn = new QPushButton("Browse...");
    m_browseBtn->setFixedWidth(90);
    m_browseBtn->setStyleSheet(R"(
        QPushButton { background:#f3f4f6; color:#374151; border:1px solid #d1d5db;
                      border-radius:5px; font-size:11px; padding:4px 8px; }
        QPushButton:hover { background:#e5e7eb; }
    )");
    connect(m_browseBtn, &QPushButton::clicked, this, &ScenarioLauncher::onBrowseClicked);

    pathRow->addWidget(m_pathLabel, 1);
    pathRow->addWidget(m_browseBtn);
    mainVL->addLayout(pathRow);

    // Separator
    auto* sep = new QFrame; sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color:#e5e7eb;");
    mainVL->addWidget(sep);

    // Main content: list + details
    auto* contentHL = new QHBoxLayout;
    contentHL->setSpacing(10);

    // Left: scenario list
    auto* leftVL = new QVBoxLayout;
    auto* listTitle = new QLabel("Scenarios");
    listTitle->setStyleSheet("font-size:12px; font-weight:700; color:#374151;");
    leftVL->addWidget(listTitle);

    m_list = new QListWidget;
    m_list->setMinimumWidth(260);
    m_list->setStyleSheet(R"(
        QListWidget {
            background: white;
            border: 1px solid #e5e7eb;
            border-radius: 6px;
            font-size: 12px;
        }
        QListWidget::item {
            padding: 7px 10px;
            border-bottom: 1px solid #f3f4f6;
        }
        QListWidget::item:selected {
            background: #eff6ff;
            color: #1d4ed8;
            font-weight: 600;
        }
        QListWidget::item:hover {
            background: #f9fafb;
        }
    )");
    connect(m_list, &QListWidget::currentRowChanged,
            this, &ScenarioLauncher::onScenarioSelected);
    leftVL->addWidget(m_list, 1);
    contentHL->addLayout(leftVL);

    // Right: description + output log
    auto* rightVL = new QVBoxLayout;
    rightVL->setSpacing(8);

    auto* descTitle = new QLabel("Description");
    descTitle->setStyleSheet("font-size:12px; font-weight:700; color:#374151;");
    rightVL->addWidget(descTitle);

    m_descLabel = new QLabel("Select a scenario to see details.");
    m_descLabel->setWordWrap(true);
    m_descLabel->setMinimumHeight(60);
    m_descLabel->setStyleSheet(R"(
        background: white;
        border: 1px solid #e5e7eb;
        border-radius: 6px;
        padding: 8px 10px;
        font-size: 12px;
        color: #4b5563;
    )");
    rightVL->addWidget(m_descLabel);

    auto* outputTitle = new QLabel("Process Output");
    outputTitle->setStyleSheet("font-size:12px; font-weight:700; color:#374151;");
    rightVL->addWidget(outputTitle);

    m_outputLog = new QTextEdit;
    m_outputLog->setReadOnly(true);
    m_outputLog->setMaximumHeight(200);
    QFont mono("Courier New");
    mono.setPointSize(10);
    m_outputLog->setFont(mono);
    m_outputLog->setStyleSheet(R"(
        QTextEdit {
            background: #1e293b;
            color: #94a3b8;
            border: 1px solid #334155;
            border-radius: 6px;
            padding: 6px;
        }
    )");
    m_outputLog->setPlaceholderText("Output will appear here when a scenario runs...");
    rightVL->addWidget(m_outputLog, 1);

    contentHL->addLayout(rightVL, 1);
    mainVL->addLayout(contentHL, 1);

    // Status + buttons row
    auto* bottomRow = new QHBoxLayout;

    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("font-size:12px; color:#6b7280;");

    m_runBtn = new QPushButton("▶  Run Scenario");
    m_runBtn->setFixedHeight(38);
    m_runBtn->setMinimumWidth(140);
    m_runBtn->setEnabled(false);
    m_runBtn->setStyleSheet(R"(
        QPushButton { background:#1d4ed8; color:white; border-radius:6px;
                      font-size:13px; font-weight:700; border:none; }
        QPushButton:hover { background:#1e40af; }
        QPushButton:disabled { background:#93c5fd; color:white; }
    )");
    connect(m_runBtn, &QPushButton::clicked, this, &ScenarioLauncher::onRunClicked);

    m_stopBtn = new QPushButton("■  Stop");
    m_stopBtn->setFixedHeight(38);
    m_stopBtn->setFixedWidth(90);
    m_stopBtn->setEnabled(false);
    m_stopBtn->setStyleSheet(R"(
        QPushButton { background:#ef4444; color:white; border-radius:6px;
                      font-size:13px; font-weight:700; border:none; }
        QPushButton:hover { background:#dc2626; }
        QPushButton:disabled { background:#fca5a5; color:white; }
    )");
    connect(m_stopBtn, &QPushButton::clicked, this, &ScenarioLauncher::onStopClicked);

    bottomRow->addWidget(m_statusLabel, 1);
    bottomRow->addWidget(m_runBtn);
    bottomRow->addWidget(m_stopBtn);
    mainVL->addLayout(bottomRow);

    // Load scenarios if path found
    if (!m_scenarioDir.isEmpty() && QDir(m_scenarioDir).exists())
        loadScenarios(m_scenarioDir);
    else
        m_list->addItem("(No scenario folder found — use Browse)");
}

ScenarioLauncher::~ScenarioLauncher() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

// ── Load all .json files from scenarios/ directory ─────────────────────────
void ScenarioLauncher::loadScenarios(const QString& dir) {
    m_scenarios.clear();
    m_list->clear();

    QDir d(dir);
    QStringList files = d.entryList({"*.json"}, QDir::Files, QDir::Name);

    for (const QString& fname : files) {
        ScenarioEntry entry;
        entry.filePath = d.absoluteFilePath(fname);

        // Read name/description from JSON
        QFile f(entry.filePath);
        if (f.open(QIODevice::ReadOnly)) {
            auto doc = QJsonDocument::fromJson(f.readAll());
            auto obj = doc.object();
            entry.name        = obj.value("name").toString(fname);
            entry.description = obj.value("description").toString("No description available.");
            f.close();
        } else {
            entry.name = fname;
        }

        m_scenarios.append(entry);

        // Color-coded icon based on scenario name/type
        QString icon = "▷ ";
        QString lower = entry.name.toLower();
        if (lower.contains("fire") || lower.contains("smoke"))   icon = "🔥 ";
        else if (lower.contains("fall"))                          icon = "🤕 ";
        else if (lower.contains("health") || lower.contains("vital")) icon = "❤ ";
        else if (lower.contains("normal") || lower.contains("reset"))  icon = "✓ ";
        else if (lower.contains("emergency"))                     icon = "🚨 ";
        else if (lower.contains("motion"))                        icon = "🚶 ";

        auto* item = new QListWidgetItem(icon + entry.name);
        item->setToolTip(entry.description);
        m_list->addItem(item);
    }

    if (m_scenarios.isEmpty())
        m_list->addItem("(No .json files found in " + dir + ")");
    else
        m_statusLabel->setText(QString("%1 scenarios loaded").arg(m_scenarios.size()));
}

// ── Slots ──────────────────────────────────────────────────────────────────
void ScenarioLauncher::onScenarioSelected() {
    int row = m_list->currentRow();
    if (row < 0 || row >= m_scenarios.size()) return;

    const auto& s = m_scenarios[row];
    m_descLabel->setText(QString("<b>%1</b><br><br>%2<br><br><span style='color:#9ca3af;font-size:10px;'>%3</span>")
        .arg(s.name).arg(s.description).arg(s.filePath));

    bool canRun = !m_testHousePath.isEmpty() && QFile::exists(m_testHousePath);
    m_runBtn->setEnabled(canRun);

    if (!canRun)
        m_statusLabel->setText("⚠ test_house executable not found — check path");
}

void ScenarioLauncher::onRunClicked() {
    int row = m_list->currentRow();
    if (row < 0 || row >= m_scenarios.size()) return;

    // Kill previous if running
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(500);
    }

    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, &QProcess::readyReadStandardOutput,
                this, &ScenarioLauncher::onProcessOutput);
        connect(m_process, &QProcess::readyReadStandardError,
                this, &ScenarioLauncher::onProcessOutput);
        connect(m_process, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
                this, &ScenarioLauncher::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred,
                this, &ScenarioLauncher::onProcessError);
    }

    m_outputLog->clear();

    const QString& scenarioFile = m_scenarios[row].filePath;
    m_process->setWorkingDirectory(QFileInfo(m_testHousePath).absolutePath());
    m_process->start(m_testHousePath, {scenarioFile});

    if (!m_process->waitForStarted(2000)) {
        m_outputLog->append("<span style='color:#ef4444;'>Failed to start test_house. "
                            "Make sure it is compiled (make) and executable.</span>");
        return;
    }

    setRunning(true);
    m_outputLog->append(QString("<span style='color:#34d399;'>▶ Started: %1 %2</span>")
                            .arg(m_testHousePath).arg(scenarioFile));
    m_statusLabel->setText("Running: " + m_scenarios[row].name);
}

void ScenarioLauncher::onStopClicked() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        if (!m_process->waitForFinished(1000))
            m_process->kill();
    }
}

void ScenarioLauncher::onBrowseClicked() {
    // Browse for test_house executable
    QString exe = QFileDialog::getOpenFileName(this,
        "Select test_house executable",
        QDir::homePath(),
        "Executables (test_house *);;All files (*)");

    if (exe.isEmpty()) return;

    m_testHousePath = exe;
    m_scenarioDir   = QFileInfo(exe).absolutePath() + "/scenarios";

    m_pathLabel->setText("📁  " + m_testHousePath);
    m_pathLabel->setStyleSheet("font-size:11px; color:#6b7280;");

    if (QDir(m_scenarioDir).exists()) {
        loadScenarios(m_scenarioDir);
    } else {
        // Browse for scenarios folder separately
        QString dir = QFileDialog::getExistingDirectory(this,
            "Select scenarios/ folder",
            QFileInfo(exe).absolutePath());
        if (!dir.isEmpty()) {
            m_scenarioDir = dir;
            loadScenarios(dir);
        }
    }
}

void ScenarioLauncher::onProcessOutput() {
    if (!m_process) return;
    QByteArray out = m_process->readAllStandardOutput();
    QByteArray err = m_process->readAllStandardError();

    if (!out.isEmpty()) {
        // Color-code output lines
        for (const QString& line : QString::fromUtf8(out).split('\n')) {
            if (line.trimmed().isEmpty()) continue;
            QString color = "#94a3b8";
            if (line.contains("ALARM") || line.contains("EMERGENCY") || line.contains("FALL"))
                color = "#f87171";
            else if (line.contains("WARNING") || line.contains("HAZARD"))
                color = "#fbbf24";
            else if (line.contains("OK") || line.contains("Connected") || line.contains("normal"))
                color = "#34d399";
            m_outputLog->append(QString("<span style='color:%1;'>%2</span>").arg(color).arg(line.toHtmlEscaped()));
        }
    }
    if (!err.isEmpty()) {
        m_outputLog->append(QString("<span style='color:#f87171;'>%1</span>")
                                .arg(QString::fromUtf8(err).toHtmlEscaped()));
    }

    // Auto-scroll to bottom
    auto* sb = m_outputLog->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void ScenarioLauncher::onProcessFinished(int exitCode, QProcess::ExitStatus) {
    setRunning(false);
    m_outputLog->append(QString("<span style='color:#6b7280;'>■ Process finished (exit code: %1)</span>")
                            .arg(exitCode));
    m_statusLabel->setText(QString("Finished (exit: %1)").arg(exitCode));
}

void ScenarioLauncher::onProcessError(QProcess::ProcessError error) {
    setRunning(false);
    QString msg;
    switch (error) {
        case QProcess::FailedToStart: msg = "Failed to start — check path and permissions (chmod +x test_house)"; break;
        case QProcess::Crashed:       msg = "Process crashed"; break;
        case QProcess::Timedout:      msg = "Timeout"; break;
        default:                      msg = "Unknown error"; break;
    }
    m_outputLog->append(QString("<span style='color:#ef4444;'>⚠ %1</span>").arg(msg));
    m_statusLabel->setText("Error: " + msg);
}

void ScenarioLauncher::setRunning(bool running) {
    m_runBtn->setEnabled(!running && m_list->currentRow() >= 0);
    m_stopBtn->setEnabled(running);
    if (!running && m_list->currentRow() >= 0)
        m_statusLabel->setText("Ready");
}
