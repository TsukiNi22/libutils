/**************************************************************\
Edition:
##  @date 27/04/2026 by @author Tsukini

File Name:
##  @file SharedObject.hpp

File Description:
##  Class used to handl warning for shared object
\**************************************************************/

#ifndef SHAREDOBJECT_H
    #define SHAREDOBJECT_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <unordered_map>    // std::unordered_map
    #include <cstdint>          // std::uint32_t
    #include <vector>           // std::vector
    #include <string>           // std::string
    #include <mutex>            // std::mutex

namespace utils::warning { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class SharedObject {
    private:
        std::mutex _mutex; // Thread safe
        bool _isSharedObject = false;
        std::string _origin = "[Unknow]"; // Path to the SharedObject
        std::unordered_map<std::uint32_t, std::string> _links;
        std::vector<std::uint32_t> _availableId;
        std::uint32_t _nextId = 0; // Can't attribute id: 0

    public:
        // ---------- Pre-Function -------- //
        std::string getOrigin(void);
        bool isSharedObject(void);
        void link(const std::string& InstanceName, std::uint32_t& id, bool safe = true);
        void unlink(std::uint32_t id, bool safe = true);

        // ------------ Operator ---------- //
        SharedObject& operator=(const SharedObject& other) = delete;
        SharedObject& operator=(SharedObject&& other) = delete;

        // ---------- Constructor --------- //
        SharedObject() noexcept;
        SharedObject(const SharedObject& other) = delete;
        SharedObject(SharedObject&& other) = delete;

        // ----------- Destructor --------- //
        ~SharedObject() noexcept;
};
} // namespace end

namespace utils::warning::WarningInstance { // namespace start
//----------------------------------------------------------------//
/* PROTOTYPE */

// Global instance declaration
extern utils::warning::SharedObject SharedObject;

} // namespace end
#endif /* SHAREDOBJECT_H */
