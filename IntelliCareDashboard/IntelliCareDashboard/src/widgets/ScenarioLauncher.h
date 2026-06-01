#pragma once

#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QProcess>
#include <QDir>
#include <QFileInfo>

class ScenarioLauncher : public QDialog {
    Q_OBJECT

public:
    explicit ScenarioLauncher(QWidget* parent = nullptr);
    ~ScenarioLauncher();

private slots:
    void onScenarioSelected();
    void onRunClicked();
    void onStopClicked();
    void onBrowseClicked();
    void onProcessOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onProcessError(QProcess::ProcessError error);

private:
    void loadScenarios(const QString& dir);
    void setRunning(bool running);

    QListWidget* m_list        = nullptr;
    QLabel*      m_descLabel   = nullptr;
    QTextEdit*   m_outputLog   = nullptr;
    QPushButton* m_runBtn      = nullptr;
    QPushButton* m_stopBtn     = nullptr;
    QPushButton* m_browseBtn   = nullptr;
    QLabel*      m_statusLabel = nullptr;
    QLabel*      m_pathLabel   = nullptr;

    QProcess*    m_process     = nullptr;
    QString      m_scenarioDir;
    QString      m_testHousePath;

    struct ScenarioEntry {
        QString filePath;
        QString name;
        QString description;
    };
    QList<ScenarioEntry> m_scenarios;
};
