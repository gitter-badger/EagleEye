#include "mainwindow.h"
#include <QApplication>

#include <Aquila/Plugins.h>
#include "MetaParameters.hpp"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

int main(int argc, char *argv[])
{
#if QT_VERSION > 0x050400
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
#endif
    QApplication a(argc, argv);

    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("log", boost::program_options::value<std::string>()->default_value("info"), "Logging verbosity. trace, debug, info, warning, error, fatal")
        ("plugins", boost::program_options::value<boost::filesystem::path>(), "Path to additional plugins to load")
        ("file", boost::program_options::value<std::string>(), "Path to file to initialize with")
        ("preferred_loader", boost::program_options::value<std::string>(), "Preferred loader to initialize with");

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);

    if(vm.count("plugins"))
    {
        auto currentDir = boost::filesystem::path(vm["plugins"].as<boost::filesystem::path>());
        boost::filesystem::directory_iterator end_itr;
        for (boost::filesystem::directory_iterator itr(currentDir); itr != end_itr; ++itr)
        {
            if (boost::filesystem::is_regular_file(itr->path()))
            {
#ifdef _MSC_VER
                if (itr->path().extension() == ".dll")
#else
                if (itr->path().extension() == ".so")
#endif
                {
                    std::string file = itr->path().string();
                    aq::loadPlugin(file);
                }
            }
        }
    }
    mo::MetaParameters::initialize();
    boost::filesystem::path currentDir = boost::filesystem::path(argv[0]).parent_path();
#ifdef _MSC_VER
    currentDir = boost::filesystem::path(currentDir.string());
#else
    currentDir = boost::filesystem::path(currentDir.string() + "/Plugins");
#endif
    LOG(info) << "Looking for plugins in: " << currentDir.string();
    boost::filesystem::directory_iterator end_itr;
    if(boost::filesystem::is_directory(currentDir))
    {
        for(boost::filesystem::directory_iterator itr(currentDir); itr != end_itr; ++itr)
        {
            if(boost::filesystem::is_regular_file(itr->path()))
            {
#ifdef _MSC_VER
                if(itr->path().extension() == ".dll")
#else
                if(itr->path().extension() == ".so")
#endif
                {
                    std::string file = itr->path().string();
                    mo::MetaObjectFactory::Instance()->LoadPlugin(file);
                }
            }
        }
    }
    mo::MetaObjectFactory::Instance()->RegisterTranslationUnit();
    MainWindow w;
    
    w.show();

    return a.exec();
}

