// Qt Thread Utilities: posixthread.cpp
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

#include <pthread.h>
#include <QDebug>
#include <QCoreApplication>
#include "posixthread.h"

POSIXThread::POSIXThread(QObject *parent) : QThread(parent),
  mScheduler(SchedDefault),
  mPriority(0),
  mThreadId(0)
{
  connect(this, SIGNAL(finished()), this, SLOT(reset()));
}

void POSIXThread::setThreadParams(const Scheduler scheduler, const quint8 priority)
{
  locker.lock();
  mScheduler = scheduler;
  mPriority = priority;
  locker.unlock();
  if (isRunning())
    setThreadParams();
}

void POSIXThread::run()
{
  setThreadParams();
  QThread::run();
}

POSIXThread::Error POSIXThread::lastError() const
{
  QMutexLocker lock(&locker);
  return mLastError;
}

void POSIXThread::setThreadParams()
{
  QMutexLocker lock(&locker);
  if (mThreadId == 0)
    {
      if (QThread::currentThread() == this)
        mThreadId = reinterpret_cast<pthread_t>(QThread::currentThreadId());
      else
        {
          // Should never get here. Ignore anyway.
          return;
        }
    }
  sched_param schedParam {mPriority};
  int result = pthread_setschedparam(mThreadId, mScheduler, &schedParam);
  lock.unlock();
  QString message("");
  switch (result)
    {
    case 0:
      break;
    case EPERM:
      message = tr("Failed to set thread policy: Operation not permitted");
      break;
    case EINVAL:
      message = tr("Failed to set thread policy: Incorrect scheduler or priority value");
      break;
    case ESRCH:
      message = tr("Failed to set thread policy: The thread is not found");
      break;
    default:
      message = tr("Failed to set thread policy: the error code is %1").arg(result);
      break;
    }
  lock.relock();
  mLastError.setError(result, message);
}

void POSIXThread::reset()
{
  QMutexLocker lock(&locker);
  mThreadId = 0;
  mScheduler = SchedDefault;
  mPriority = 0;
}
