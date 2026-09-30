#ifndef TERMINAL_H
#define TERMINAL_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QPoint>
#include <QStringList>

// Frameless command console with custom title bar.
// Commands: help, about, gi, dlg, ws, cs, ds, blog, thoughts, cv, color, hints, exit.
class Terminal : public QWidget {
    Q_OBJECT

public:
    explicit Terminal(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private slots:
    void onCommandEntered();

private:
    void appendOutput(const QString &text, const QString &color = QString());
    void scrollToBottom();
    void printBanner();
    void processCommand(const QString &cmd);
    void navigateHistory(int direction);
    void autocomplete();
    void toggleHints();
    void applyTheme();

    void cmdHelp();
    void cmdAbout();
    void cmdGameInfo();
    void cmdDownloadGame();
    void cmdWebsite();
    void cmdListSaves();
    void cmdDeleteSave(const QString &name);
    void cmdBlog();
    void cmdThoughts();
    void cmdCheckVersion();
    void cmdColor();

    QString savesPath() const;

    QWidget     *m_titleBar   = nullptr;
    QLabel      *m_titleLabel = nullptr;
    QPushButton *m_closeBtn   = nullptr;
    QTextEdit   *m_output     = nullptr;
    QWidget     *m_inputRow   = nullptr;
    QLabel      *m_prompt     = nullptr;
    QLineEdit   *m_input      = nullptr;
    QLabel      *m_hint       = nullptr;

    bool   m_dragging = false;
    QPoint m_dragPos;

    QStringList m_history;
    int         m_historyIndex = -1;

    const QStringList m_commands = {
        "about", "help", "gi", "dlg", "ws", "cs", "ds", "blog",
        "thoughts", "cv", "color", "clear", "exit", "hints"
    };

    bool m_awaitingSaveName = false;

    bool m_darkTheme = true;
    QString m_colText, m_colMuted, m_colError, m_colSuccess, m_colAccent, m_colDivider;
};

#endif // TERMINAL_H