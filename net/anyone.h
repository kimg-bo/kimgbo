#ifndef OTHER_ANY_H
#define OTHER_ANY_H

#include <algorithm>
#include <typeinfo>

class any
{
public: // structors
    any()
      : content(0)
    {
    }

    template<typename ValueType>
    any(const ValueType & value)
      : content(new holder<ValueType>(value))
    {
    }

    any(const any & other)
      : content(other.content ? other.content->clone() : 0)
    {
    }

    ~any()
    {
        delete content;
    }

public: // modifiers
    any & swap(any & rhs)
    {
        std::swap(content, rhs.content);
        return *this;
    }

    template<typename ValueType>
    any & operator=(const ValueType & rhs)
    {
        any(rhs).swap(*this);
        return *this;
    }

    any & operator=(const any & rhs)
    {
        any(rhs).swap(*this);
        return *this;
    }

public: // queries
    bool empty() const
    {
        return !content;
    }

    const std::type_info & type() const
    {
        return content ? content->type() : typeid(void);
    }

public: // types (public so any_cast can be non-friend)
    class placeholder
    {
    public: // structors
        virtual ~placeholder()
        {
        }

    public: // queries
        virtual const std::type_info & type() const = 0;
        virtual placeholder * clone() const = 0;
    };

    template<typename ValueType>
    class holder : public placeholder
    {
    public: // structors
        holder(const ValueType & value)
          : held(value)
        {
        }

    public: // queries
        virtual const std::type_info & type() const
        {
            return typeid(ValueType);
        }

        virtual placeholder * clone() const
        {
            return new holder(held);
        }

    public: // representation
        ValueType held;
    };

private: // representation
    template<typename ValueType>
    friend ValueType * any_cast(any *);
        
public:
    placeholder* content;
};

class bad_any_cast : public std::bad_cast
{
public:
    virtual const char * what() const throw()
    {
        return "bad_any_cast: "
               "failed conversion using any_cast";
    }
};

template<typename ValueType>
ValueType * any_cast(any * operand)
{
    return operand && operand->type() == typeid(ValueType)
                ? &static_cast<any::holder<ValueType> *>(operand->content)->held
                : 0;
}

template<typename ValueType>
const ValueType* any_cast(const any* operand)
{
    return any_cast<ValueType>(const_cast<any*>(operand));
}

template<typename ValueType>
ValueType any_cast(const any & operand)
{
    const ValueType* result = any_cast<ValueType>(&operand);
    if(!result)
        throw bad_any_cast();
    return *result;
}

// Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#endif //OTHER_ANY_H
