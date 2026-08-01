/**************************************************************\
Edition:
##  @date 01/08/2026 by @author Tsukini

File Name:
##  @file IObserver.hpp

File Description:
##  Interface of the different observers
\**************************************************************/

#ifndef IOBSERVER_H
    #define IOBSERVER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <string>   // std::string

namespace utils::security::observer { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class IObserver {
    protected:
        // ---------- Pre-Function -------- //
        virtual void link(void) = 0;
        virtual void unlink(void) = 0;

    public:
        // ------------ Operator ---------- //
        IObserver& operator=(const IObserver& other) = default;
        IObserver& operator=(IObserver&& other) = default;

        // ---------- Constructor --------- //
        IObserver() = default;
        IObserver(const IObserver& other) = default;
        IObserver(IObserver&& other) = default;

        // ----------- Destructor --------- //
        virtual ~IObserver() = default;
};

} // namespace end
#endif /* IOBSERVER_H */
