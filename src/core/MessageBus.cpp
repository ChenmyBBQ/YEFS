#include "MessageBus.h"
#include <QDebug>

namespace YEFS {

MessageBus* MessageBus::s_instance = nullptr;

MessageBus::MessageBus(QObject* parent)
    : QObject(parent)
{
}

MessageBus* MessageBus::instance()
{
    if (!s_instance) {
        s_instance = new MessageBus();
    }
    return s_instance;
}

MessageBus* MessageBus::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

void MessageBus::publish(const QString& topic, const QVariant& data)
{
    // 发送全局信号
    emit message(topic, data);

    // 通知所有订阅者
    auto subscribers = m_subscribers.values(topic);
    for (const auto& subscriber : subscribers) {
        if (subscriber.first) {
            QMetaObject::invokeMethod(subscriber.first, subscriber.second,
                                      Qt::AutoConnection,
                                      Q_ARG(QString, topic),
                                      Q_ARG(QVariant, data));
        }
    }
}

void MessageBus::subscribe(const QString& topic, QObject* receiver, const char* slot)
{
    if (!receiver || !slot) return;

    m_subscribers.insert(topic, qMakePair(receiver, slot));

    // 当接收者销毁时自动取消订阅
    connect(receiver, &QObject::destroyed, this, [this, receiver]() {
        unsubscribeAll(receiver);
    });
}

void MessageBus::unsubscribe(const QString& topic, QObject* receiver)
{
    auto it = m_subscribers.find(topic);
    while (it != m_subscribers.end() && it.key() == topic) {
        if (it.value().first == receiver) {
            it = m_subscribers.erase(it);
        } else {
            ++it;
        }
    }
}

void MessageBus::unsubscribeAll(QObject* receiver)
{
    auto it = m_subscribers.begin();
    while (it != m_subscribers.end()) {
        if (it.value().first == receiver) {
            it = m_subscribers.erase(it);
        } else {
            ++it;
        }
    }
}

void MessageBus::send(const QString& topic, const QVariant& data)
{
    publish(topic, data);
}

} // namespace YEFS
