/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 30/07/2026 by @author Tsukini

File Name:
##  @file IKey.hpp

File Description:
##  Declaration of the interface used for different key (RSA, AES, ...)
\**************************************************************/

#ifndef IKEY_H
    #define IKEY_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <cstdint>  // std::uint16_t
    #include <string>   // std::string

namespace utils::smanip::key { // namespace start
//----------------------------------------------------------------//
/* CLASS */

template<typename T>
class IKey {
    public:
        // ---------- Pre-Function -------- //
        virtual std::string generateRandomBytes(std::uint16_t size = 32) const = 0;
        virtual void generate(void) = 0;
        virtual void set(const T& data) = 0;
        virtual std::string encrypt(const std::string& s) const = 0;
        virtual std::string decrypt(const std::string& s) const = 0;
        virtual std::string encrypt(const std::string& s, T& data) const = 0;
        virtual std::string decrypt(const std::string& s, T& data) const = 0;
        virtual bool hasGenerateOverload(void) const = 0;
        virtual bool hasSetOverload(void) const = 0;

        // ------------ Operator ---------- //
        IKey& operator=(const IKey& other) = delete;
        IKey& operator=(IKey&& other) = delete;

        // ---------- Constructor --------- //
        IKey() = default;
        IKey(const IKey& other) = delete;
        IKey(IKey&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~IKey() = default;
};

} // namespace end
#endif /* IKEY_H */
