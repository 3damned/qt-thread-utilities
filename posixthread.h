// Qt Thread Utilities: posixthread.h
/***************************************************************************
 *   Copyright (C) 2015-2015 Ilya Kuznietsov, <3damned@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/*
  POSIXThread provides priority and scheduler managing for QThread based
  on POSIX thread. See pthread_setschedparam() description for details.
*/

#ifndef POSIXTHREAD_H
#define POSIXTHREAD_H

#include <QThread>
#include <QMutex>

class POSIXThread : public QThread
{
  Q_OBJECT
public:
  enum Scheduler {
    SchedDefault = 0,
    SchedFIFO = 1,
    SchedRoundRobin = 2
  };

  class Error
  {
  public:
    Error(int code, QString text) { setError(code, text); }
#ifdef Q_COMPILER_DELEGATING_CONSTRUCTORS
    Error() : Error(0, "") {}
#else
    Error() { mCode(0); mText(""); }
#endif
    int code() const { return mCode; }
    QString text() const { return mText; }
    void setError(int code, QString text = "") { mCode = code; mText = text; }

    private:
      int mCode;
      QString mText;
  };

  // Construct / destroy
  POSIXThread(QObject *parent = NULL);
  ~POSIXThread() {}

/*
  Sets scheduler and priority for pthread managed by this POSIXThread object.
  If the thread is not running the parameters will be applied as soon as
  the thread start, otherwise the parameters will be applied immediately.
  You may need superuser rights to apply SchedFIFO or SchedRoundRobin policy.
*/
  void setThreadParams(const Scheduler scheduler, const quint8 priority);

/*
  Reimplemented from QThread::run. Starts the thread's event loop.
*/
  void run() Q_DECL_FINAL;

/*
  Provides the result of the most recent policy change try. 0 means no error.
*/
  Error lastError() const;

private:
  void setThreadParams();
  Q_SLOT void reset();

  Scheduler mScheduler;
  quint8 mPriority;
  pthread_t mThreadId;
  Error mLastError;
  mutable QMutex locker;
};

#endif // POSIXTHREAD_H
