/**************************************************************\
Edition:
##  @date 30/07/2026 by @author Tsukini

File Name:
##  @file Observer.cpp

File Description:
##  Warning observer methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/security/observer/Observer.hpp"

void utils::security::observer::Observer::link(void)
{
    utils::security::observer::WarningInstance::SharedObject.link(this->getInstanceName(), this->_id);
}

void utils::security::observer::Observer::unlink(void)
{
    utils::security::observer::WarningInstance::SharedObject.unlink(this->_id);
}

utils::security::observer::Observer& utils::security::observer::Observer::operator=(_unused const Observer& other)
{
    return *this;
}

utils::security::observer::Observer& utils::security::observer::Observer::operator=(Observer&& other)
{
    this->unlink();
    this->_id = other._id;
    other._id = 0;
    return *this;
}

utils::security::observer::Observer::Observer()
{
    this->link();
}

utils::security::observer::Observer::Observer(_unused const Observer& other)
{
    this->link();
}

utils::security::observer::Observer::Observer(Observer&& other)
: _id{other._id}
{
    other._id = 0;
}

utils::security::observer::Observer::~Observer()
{
    this->unlink();
}
