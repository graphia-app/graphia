#ifndef SINGLETON_H
#define SINGLETON_H

#include <QtGlobal>

template<typename T> class Singleton // NOLINT
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

// Allows access to a singleton via S(Class)->... instead of Class::instance()->...
#define S(X) X::instance() /* NOLINT cppcoreguidelines-macro-usage */

#endif
