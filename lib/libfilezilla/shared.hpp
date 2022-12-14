#ifndef LIBFILEZILLA_SHARED_HEADER
#define LIBFILEZILLA_SHARED_HEADER

#include <memory>

/** \file
* \brief Declares the shared_optional and shared_value template classes.
*/
namespace fz {

/** \brief shared_optional is like std::shared_ptr but with relational operators acting like
 * C++17's std::optional
 *
 * \tparam T that can be stored in the shared_optional
 * \tparam Init controls whether the optional can be empty. If \c false, the default,
 *         it acts like a normal optional/shared_ptr. If \c true, it can never be
 *         empty. \see shared_value
 *
 * This class is thread-safe under the following assumptions:
 * - The type stored in it must be thread-safe for reading
 * - Any shared_optional instance a non-const member is called on (e.g. get())
 *   must not be used in different threads concurrently.
 */
template<typename T, bool Init = false> class shared_optional final
{
public:
	shared_optional() noexcept(!Init || std::is_nothrow_constructible_v<T>);
	shared_optional(shared_optional<T, Init> const& v) = default;
	shared_optional(shared_optional<T, Init> && v) noexcept = default;
	explicit shared_optional(const T& v);

	void clear();

	/**
	 * \brief Gets a reference to the stored object.
	 *
	 * A new object is created it if is not set.
	 * Returned reference is unique, it is not shared.
	 */
	T& get();

	const T& operator*() const;
	const T* operator->() const;

	/**
	 * \name Deep relational operators
	 *
	 * If two instances point to
	 * different objects, those objects are compared.
	 * Empty instances are less than non-empty instances.
	 * \{
	 */
	bool operator==(shared_optional<T, Init> const& cmp) const;
	bool operator==(T const& cmp) const;
	bool operator<(shared_optional<T, Init> const& cmp) const;
	bool operator<(T const& cmp) const;

	/// Only compares the pointers
	bool is_same(shared_optional<T, Init> const& cmp) const;

	inline bool operator!=(const shared_optional<T, Init>& cmp) const { return !(*this == cmp); }
	inline bool operator!=(T const& cmp) const { return !(*this == cmp); }
	/// \}

	shared_optional<T, Init>& operator=(shared_optional<T, Init> const& v) = default;
	shared_optional<T, Init>& operator=(shared_optional<T, Init> && v) noexcept = default;

	explicit operator bool() const { return static_cast<bool>(data_); }

	bool empty() const { return !data_; }
private:
	std::shared_ptr<T> data_;
};

/** \brief like shared_optional but can never be empty
 *
 * All operations that would result in an empty shared_optional instead
 * result in a default-constructed value.
 *
 * As such, operator* and operator-> are always well-defined.
 */
template<typename T>
using shared_value = shared_optional<T, true>;


template<typename T, bool Init> shared_optional<T, Init>::shared_optional() noexcept(!Init || std::is_nothrow_constructible_v<T>)
    : data_(Init ? std::make_shared<T>() : nullptr)
{
}

template<typename T, bool Init> shared_optional<T, Init>::shared_optional(const T& v)
	: data_(std::make_shared<T>(v))
{
}

template<typename T, bool Init> bool shared_optional<T, Init>::operator==(shared_optional<T, Init> const& cmp) const
{
	if (data_ == cmp.data_) {
		return true;
	}
	else if (!Init && (!data_ || !cmp.data_)) {
		return false;
	}

	return *data_ == *cmp.data_;
}

template<typename T, bool Init> bool shared_optional<T, Init>::operator==(T const& cmp) const
{
	if (!Init && !data_) {
		return false;
	}
	return *data_ == cmp;
}

template<typename T, bool Init> bool shared_optional<T, Init>::is_same(shared_optional<T, Init> const& cmp) const
{
	return data_ == cmp.data_;
}

template<typename T, bool Init> T& shared_optional<T, Init>::get()
{
	if (!Init && !data_) {
		data_ = std::make_shared<T>();
	}
	if (data_.use_count() > 1) {
		data_ = std::make_shared<T>(*data_);
	}

	return *data_;
}

template<typename T, bool Init> bool shared_optional<T, Init>::operator<(shared_optional<T, Init> const& cmp) const
{
	if (data_ == cmp.data_) {
		return false;
	}
	else if (!Init && !data_) {
		return static_cast<bool>(cmp.data_);
	}
	else if (!Init && !cmp.data_) {
		return false;
	}
	return *data_ < *cmp.data_;
}

template<typename T, bool Init> bool shared_optional<T, Init>::operator<(T const& cmp) const
{
	if (!Init && !data_) {
		return true;
	}
	return *data_ < cmp;
}

template<typename T, bool Init> void shared_optional<T, Init>::clear()
{
	if (!Init) {
		data_.reset();
	}
	else if (data_.use_count() <= 1) {
		*data_ = T();
	}
	else {
		data_ = std::make_shared<T>();
	}
}

template<typename T, bool Init> const T& shared_optional<T, Init>::operator*() const
{
	return *data_;
}

template<typename T, bool Init> const T* shared_optional<T, Init>::operator->() const
{
	return data_.get();
}

}

#endif
