// Qt Thread Utilities: threaded.h
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
  Threaded<T> class is intended for a convenient object execution in a thread.
  It takes care about the object deletion as well as related QThread object
  (in case of no parent provided for QThread).
  Class T must be based on QObject.
  See details in QSharedPointer<T> class documentation.
*/
  
#ifndef THREADED_H
#define THREADED_H

#include <QThread>
#include <QDebug>
#include <QSharedPointer>

template <typename T>
class Threaded : public QSharedPointer<T>
{
public:
  // Construct / destroy
  Threaded(T *object, QThread *owner);
  Threaded();
  ~Threaded();

  // Start the thread provided in the constructor
  void runThreaded();
  // Stop the thread
  void quitThread();

private:
  QThread *mThread;
  bool mHasOwnThread;
};

// Implementation
template <typename T>
Threaded<T>::Threaded(T *object, QThread *owner) : QSharedPointer<T>(object)
{
#ifdef Q_COMPILER_STATIC_ASSERT
  static_assert(std::is_base_of<QObject, T>::value, "Threaded<T> error: T is not based on QObject");
#endif
  if ((mHasOwnThread = (owner != NULL) && (owner != QThread::currentThread())))
    {
      mThread = owner;
      if (QObject *thisThreadObj = qobject_cast<QObject *>(object))
        mThread->setObjectName(thisThreadObj->metaObject()->className());
    }
  else mThread = QThread::currentThread();
}

#ifdef Q_COMPILER_DELEGATING_CONSTRUCTORS
template <typename T>
Threaded<T>::Threaded() : Threaded<T>::Threaded(NULL, NULL)
{

}
#else
template <typename T>
Threaded<T>::Threaded() : QSharedPointer<T>(NULL),
  mThread(QThread::currentThread()),
  mHasOwnThread(false)
{

}
#endif

template <typename T>
Threaded<T>::~Threaded()
{
#ifdef THREADED_TESTS
  qDebug() << "Threaded" << QString("%1 destroyed").arg(this->data()->metaObject()->className());
#endif
  if (mHasOwnThread)
    {
      quitThread();
      if (!mThread->parent())
        delete mThread;
    }
}

template <typename T>
void Threaded<T>::runThreaded()
{
  QObject *thisObj = qobject_cast<QObject *>(this->data());
  if (thisObj)
    {
      if (thisObj->thread() != mThread)
        {
          QObject *thisObjParent = thisObj->parent();
          thisObj->setParent(NULL);
          thisObj->moveToThread(mThread);
          if (thisObjParent != NULL && mThread == thisObjParent->thread())
            thisObj->setParent(thisObjParent);
          mThread->start();
        }
      else
        qWarning() << QObject::tr("Thread for %1 is already running").arg(thisObj->metaObject()->className());
    }
  else
    qCritical() << QObject::tr("Class T must be based on QObject");
}

template <typename T>
void Threaded<T>::quitThread()
{
  mThread->quit();
  if (mThread->isRunning())
    if (!mThread->wait(3000))
      {
#ifdef THREADED_TESTS
        qDebug() << QString("%1 thread termination requested").arg(this->data()->metaObject()->className());
#endif
        mThread->terminate();
        mThread->wait(3000);
      }
}


#endif // THREADED_H
