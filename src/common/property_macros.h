#pragma once

#define DECLARE_PROPERTY_UPDATE(Type, name, Name, Signal, updateFunc) \
private: \
    Type m_##name{}; \
public: \
    Q_PROPERTY(Type name READ name WRITE set##Name NOTIFY Signal) \
    Type name() const { \
        return m_##name; \
    } \
    void set##Name(Type value) { \
        if (m_##name == value) \
            return; \
        m_##name = value; \
        emit Signal(); \
        updateFunc(); \
    }
