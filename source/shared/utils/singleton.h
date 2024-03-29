/* Copyright © 2013-2024 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SINGLETON_H
#define SINGLETON_H

#include <QtGlobal>

template<typename T> class Singleton
{
public:
  Singleton()
  {
    Q_ASSERT(_singletonPtr == nullptr);
    _singletonPtr = this;
  }

  virtual ~Singleton()
  {
    Q_ASSERT(_singletonPtr != nullptr);
    _singletonPtr = nullptr;
  }

  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;

  static T* instance()
  {
      // An instance of T needs to be created somewhere
      Q_ASSERT(_singletonPtr != nullptr);

      return static_cast<T*>(_singletonPtr);
  }

private:
  static Singleton<T>* _singletonPtr;
};

template<typename T> Singleton<T>* Singleton<T>::_singletonPtr = nullptr;

#endif
