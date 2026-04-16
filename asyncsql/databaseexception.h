#ifndef DATABASEEXCEPTION_H
#define DATABASEEXCEPTION_H

#include <exception>
#include <QString>
#include <QSqlError>

namespace AsyncSql {
class DatabaseException : std::exception
{
public:
    enum ErrorCode {FailedToCommit = 10000, DuplicateEntry};
    DatabaseException(const QSqlError &e, const QString userMsg = QString()) :
        error(e),
        code(static_cast<int>(e.type())),
        msg(e.text())
    {
        if(userMsg.isEmpty())
            this->userMsg = msg;
    }

    const char *what() const noexcept {
        whatBuffer = QStringLiteral("Error %1: %2").arg(code).arg(msg).toStdString();
        return whatBuffer.c_str();
    }

    QSqlError getError() const { return error; }
    ~DatabaseException() noexcept {}
private:
    int code;
    QString msg;
    QString userMsg;
    QSqlError error;
    mutable std::string whatBuffer;
};
}

#endif // DATABASEEXCEPTION_H
