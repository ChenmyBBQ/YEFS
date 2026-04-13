#include "NerdIcon.h"

namespace YEFS {

NerdIcon *NerdIcon::s_instance = nullptr;

NerdIcon *NerdIcon::instance()
{
    if (!s_instance)
        s_instance = new NerdIcon();
    return s_instance;
}

NerdIcon *NerdIcon::create(QQmlEngine * /*qmlEngine*/, QJSEngine * /*jsEngine*/)
{
    auto *inst = instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

void NerdIcon::destroy()
{
    delete s_instance;
    s_instance = nullptr;
}

} // namespace YEFS
