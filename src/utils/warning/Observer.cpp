/**************************************************************\
Edition:
##  @date 27/04/2026 by @author Tsukini

File Name:
##  @file Observer.cpp

File Description:
##  Warning observer methods
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/warning/Observer.hpp"

void utils::warning::Observer::link(void)
{
    #ifndef NO_SHARED_OBJECT_WARNING
        utils::warning::WarningInstance::SharedObject.link(this->getInstanceName(), this->_id);
    #endif
}

void utils::warning::Observer::unlink(void)
{
    #ifndef NO_SHARED_OBJECT_WARNING
        utils::warning::WarningInstance::SharedObject.unlink(this->_id);
    #endif
}

utils::warning::Observer& utils::warning::Observer::operator=(unused const Observer& other)
{
    return *this;
}

utils::warning::Observer& utils::warning::Observer::operator=(Observer&& other)
{
    this->unlink();
    this->_id = other._id;
    other._id = 0;
    return *this;
}

utils::warning::Observer::Observer()
{
    this->link();
}

utils::warning::Observer::Observer(unused const Observer& other)
{
    this->link();
}

utils::warning::Observer::Observer(Observer&& other)
: _id{other._id}
{
    other._id = 0;
}

utils::warning::Observer::~Observer()
{
    this->unlink();
}
