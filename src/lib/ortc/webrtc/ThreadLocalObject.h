#ifndef TGCALLS_THREAD_LOCAL_OBJECT_H
#define TGCALLS_THREAD_LOCAL_OBJECT_H

#include <cassert>
#include <functional>
#include <memory>
#include <iostream>

#include <ok-rtc/rtc_base/thread.h>
#include <ok-rtc/rtc_base/location.h>

namespace lib::ortc {

template <typename T>
class ThreadLocalObject {
public:
	template <
		typename Generator,
		typename = std::enable_if_t<std::is_same<T*, decltype(std::declval<Generator>()())>::value>>
	ThreadLocalObject(rtc::Thread *thread, Generator &&generator) :
	_thread(thread),
	_valueHolder(std::make_unique<ValueHolder>()) {
		assert(_thread != nullptr);
        _thread->PostTask([valueHolder = _valueHolder.get(),
                          generator = std::forward<Generator>(generator)]()
                          mutable {
			valueHolder->_value.reset(generator());
		});
	}

	~ThreadLocalObject() {
		_thread->PostTask([valueHolder = std::move(_valueHolder)](){
			valueHolder->_value.reset();
		});
	}

	template <typename FunctorT>
	void perform(const rtc::Location& posted_from, FunctorT &&functor) {
		_thread->PostTask([valueHolder = _valueHolder.get(), f = std::forward<FunctorT>(functor)]() mutable {
			assert(valueHolder->_value != nullptr);
			f(valueHolder->_value.get());
		});
	}

	T *getSyncAssumingSameThread() {
        assert(_thread);
        std::cout << "thread:" << _thread->name() << std::endl;
		assert(_thread->IsCurrent());
		assert(_valueHolder->_value != nullptr);
		return _valueHolder->_value.get();
	}

private:
	struct ValueHolder {
		std::shared_ptr<T> _value;
	};

	rtc::Thread *_thread = nullptr;
	std::unique_ptr<ValueHolder> _valueHolder;

};

} // namespace lib::ortc

#endif
