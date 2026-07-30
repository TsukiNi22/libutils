/**************************************************************\
Edition:
##  @date 05/07/2026 by @author Tsukini

File Name:
##  @file UnsafeObserver.cpp

File Description:
##  Warning observer methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/security/observer/UnsafeObserver.hpp"

void utils::security::observer::UnsafeObserver::link(void)
{
    utils::security::observer::WarningInstance::SharedObject.link(this->getInstanceName(), this->_id, false);
}

void utils::security::observer::UnsafeObserver::unlink(void)
{
    utils::security::observer::WarningInstance::SharedObject.unlink(this->_id, false);
}

utils::security::observer::UnsafeObserver& utils::security::observer::UnsafeObserver::operator=(_unused const UnsafeObserver& other)
{
    return *this;
}

utils::security::observer::UnsafeObserver& utils::security::observer::UnsafeObserver::operator=(UnsafeObserver&& other)
{
    this->unlink();
    this->_id = other._id;
    other._id = 0;
    return *this;
}

utils::security::observer::UnsafeObserver::UnsafeObserver()
{
    this->link();
}

utils::security::observer::UnsafeObserver::UnsafeObserver(_unused const UnsafeObserver& other)
{
    this->link();
}

utils::security::observer::UnsafeObserver::UnsafeObserver(UnsafeObserver&& other)
: _id{other._id}
{
    other._id = 0;
}

utils::security::observer::UnsafeObserver::~UnsafeObserver()
{
    this->unlink();
}
