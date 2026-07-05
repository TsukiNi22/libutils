/**************************************************************\
Edition:
##  @date 05/07/2026 by @author Tsukini

File Name:
##  @file UnsafeObserver.cpp

File Description:
##  Warning observer methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/warning/UnsafeObserver.hpp"

void utils::warning::UnsafeObserver::link(void)
{
    utils::warning::WarningInstance::SharedObject.link(this->getInstanceName(), this->_id, false);
}

void utils::warning::UnsafeObserver::unlink(void)
{
    utils::warning::WarningInstance::SharedObject.unlink(this->_id, false);
}

utils::warning::UnsafeObserver& utils::warning::UnsafeObserver::operator=(unused const UnsafeObserver& other)
{
    return *this;
}

utils::warning::UnsafeObserver& utils::warning::UnsafeObserver::operator=(UnsafeObserver&& other)
{
    this->unlink();
    this->_id = other._id;
    other._id = 0;
    return *this;
}

utils::warning::UnsafeObserver::UnsafeObserver()
{
    this->link();
}

utils::warning::UnsafeObserver::UnsafeObserver(unused const UnsafeObserver& other)
{
    this->link();
}

utils::warning::UnsafeObserver::UnsafeObserver(UnsafeObserver&& other)
: _id{other._id}
{
    other._id = 0;
}

utils::warning::UnsafeObserver::~UnsafeObserver()
{
    this->unlink();
}
