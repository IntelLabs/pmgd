#pragma once

namespace Jarvis {
    template <typename KeyType, typename ValueType> class KeyValuePair {
        KeyType _key;
        ValueType _value;

    public:
        KeyValuePair(const KeyType& k, const ValueType& v)
        {
            _key = k;
            _value = v;
        }
        bool operator==(const KeyValuePair& val2) const
        {
            return (_key == val2._key);
        }
        void operator=(const long value)
        {
            _key = (KeyType)value;
            _value = (ValueType)value;
        }
        bool operator<(const KeyValuePair& val2) const
        {
            return (_key < val2._key);
        }
        void set(const KeyType &k, const ValueType &v)
        {
            _key = k;
            _value = v;
        }
        void set_key(const KeyType &k) { _key = k; }
        void set_value(const ValueType &v) { _value = v; }
        const KeyType &key() const { return _key; }
        ValueType &value() { return _value; }
        const ValueType &value() const { return _value; }
    };
}
