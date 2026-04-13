#include "AppIconManager.h"

namespace YEFS {

AppIconManager *AppIconManager::s_instance = nullptr;

AppIconManager *AppIconManager::instance()
{
    if (!s_instance)
        s_instance = new AppIconManager();
    return s_instance;
}

AppIconManager *AppIconManager::create(QQmlEngine * /*qmlEngine*/, QJSEngine * /*jsEngine*/)
{
    auto *inst = instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

void AppIconManager::destroy()
{
    delete s_instance;
    s_instance = nullptr;
}

} // namespace YEFS
