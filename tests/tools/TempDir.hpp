/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 01/10/2026 by @author Tsukini

File Name:
##  @file TempDir.hpp

File Description:
##  RAII temporary directory & environment override used by the tests
\**************************************************************/

#ifndef TEMPDIR_H
    #define TEMPDIR_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <filesystem>   // std::filesystem::path, std::filesystem::temp_directory_path
    #include <stdexcept>    // std::runtime_error
    #include <optional>     // std::optional
    #include <cstdlib>      // mkdtemp, setenv, unsetenv, getenv (_putenv_s on Windows)
    #include <random>       // std::random_device (Windows)
    #include <string>       // std::string

namespace tests::tools { // namespace start
//----------------------------------------------------------------//
/* CLASS */

// Create a unique directory at construction and remove it at destruction
class TempDir {
    private:
        std::filesystem::path _path;

    public:
        const std::filesystem::path& path(void) const {return this->_path;};
        std::filesystem::path operator/(const std::string& name) const {return this->_path / name;};

        TempDir(void)
        {
            #ifdef _WIN32
                // No mkdtemp: random name, create_directory fails if it already exists
                std::random_device rd;
                for (int i = 0; i < 100 && this->_path.empty(); ++i) {
                    std::filesystem::path path = std::filesystem::temp_directory_path() / ("utils-tests-" + std::to_string(rd()));
                    if (std::filesystem::create_directory(path)) this->_path = path;
                }
                if (this->_path.empty())
                    throw std::runtime_error("create_directory failed");
            #else
                std::string pattern = (std::filesystem::temp_directory_path() / "utils-tests-XXXXXX").string();
                if (::mkdtemp(pattern.data()) == nullptr)
                    throw std::runtime_error("mkdtemp failed");
                this->_path = pattern;
            #endif
        };
        TempDir(const TempDir& other) = delete;
        TempDir& operator=(const TempDir& other) = delete;
        ~TempDir() {std::error_code ec; std::filesystem::remove_all(this->_path, ec);};
};

// Override an environment variable and restore it at destruction
class ScopedEnv {
    private:
        std::string _name;
        std::optional<std::string> _old;

    public:
        #ifdef _WIN32
            static void set(const std::string& name, const std::string& value) {::_putenv_s(name.c_str(), value.c_str());};
            static void unset(const std::string& name)                         {::_putenv_s(name.c_str(), "");}; // empty = removed
        #else
            static void set(const std::string& name, const std::string& value) {::setenv(name.c_str(), value.c_str(), 1);};
            static void unset(const std::string& name)                         {::unsetenv(name.c_str());};
        #endif

        ScopedEnv(const std::string& name, const std::string& value): _name{name}
        {
            if (const char* old = std::getenv(name.c_str())) this->_old = old;
            set(name, value);
        };
        ScopedEnv(const ScopedEnv& other) = delete;
        ScopedEnv& operator=(const ScopedEnv& other) = delete;
        ~ScopedEnv()
        {
            if (this->_old.has_value()) set(this->_name, *this->_old);
            else unset(this->_name);
        };
};

} // namespace end
#endif /* TEMPDIR_H */
