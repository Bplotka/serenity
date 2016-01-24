#ifndef SERENITY_PYPELINE_FILTER_HPP
#define SERENITY_PYPELINE_FILTER_HPP

#include <ctime>
#include <list>
#include <memory>
#include <string>

#include <boost/python.hpp>

#include "mesos/mesos.hpp"

#include "messages/serenity.hpp"

#include "serenity/config.hpp"
#include "serenity/default_vars.hpp"
#include "serenity/serenity.hpp"

namespace mesos {
namespace serenity {

class PypelineFilterConfig : public SerenityConfig {
 public:
  PypelineFilterConfig() { }

  explicit PypelineFilterConfig(const SerenityConfig& customCfg) {
    this->initDefaults();
    this->applyConfig(customCfg);
  }

  void initDefaults() {
    this->fields[python_pipeline::SERENITY_PYPELINE_PATH] =
      (std::string) python_pipeline::DEFAULT_SERENITY_PYPELINE_PATH;
    this->fields[python_pipeline::SERENITY_PYPELINE_MODULE] =
      (std::string) python_pipeline::DEFAULT_SERENITY_PYPELINE_MODULE;
    this->fields[python_pipeline::SERENITY_PYPELINE_CLASS] =
      (std::string) python_pipeline::DEFAULT_SERENITY_PYPELINE_CLASS;
  }
};


/**
 * Initializing python interpreter and runnnig `serenity-pypeline`.
 * https://github.com/Bplotka/serenity-pypeline
 */
class PypelineFilter :
  public Consumer<ResourceUsage>, public Producer<Contentions> {
 public:
  explicit PypelineFilter(const Tag& _tag = Tag(QOS_CONTROLLER, NAME))
    : tag(_tag) {}

  explicit PypelineFilter(
      Consumer<Contentions>* _consumer,
      SerenityConfig _conf,
      const Tag& _tag = Tag(QOS_CONTROLLER, NAME))
      : Producer<Contentions>(_consumer), tag(_tag) {
    SerenityConfig config = PypelineFilterConfig(_conf);
    cfgSerenityPypelinePath =
      config.getS(python_pipeline::SERENITY_PYPELINE_PATH);

    cfgSerenityPypelineModule =
      config.getS(python_pipeline::SERENITY_PYPELINE_MODULE);

    cfgSerenityPypelineClass =
      config.getS(python_pipeline::SERENITY_PYPELINE_CLASS);

    initializePythonInterpreter();
  }

  ~PypelineFilter() {
    Py_Finalize();
  }

  static const constexpr char* NAME = "PypelineFilter";

  Try<Nothing> consume(const ResourceUsage& in) {
    if (pyInstance == nullptr) {
      return Error("Class from path " + cfgSerenityPypelinePath
                   + " not initialized.");
    }

    // Continue pipeline. Currently, we won't receive any automatic
    // scheduling actions.
    produce(Contentions());

    return Nothing();
  }

 private:
  const Tag tag;

  boost::python::object pyModule, pyInstance;

  std::string cfgSerenityPypelinePath;
  std::string cfgSerenityPypelineModule;
  std::string cfgSerenityPypelineClass;

  void initializePythonInterpreter() {
    // Initialize Python interpreter.
    Py_Initialize();

    // Configure sys path.
    PyRun_SimpleString("import sys\n");
    PyRun_SimpleString(std::string("sys.path.append('" +
                         cfgSerenityPypelinePath + "')\n").c_str());

    try {

      pyModule = boost::python::import(cfgSerenityPypelineModule.c_str());

      pyInstance = pyModule.attr(cfgSerenityPypelineClass.c_str())();

    } catch (...) {
      PyErr_Print();
      PyErr_Clear();
      SERENITY_LOG(ERROR) << "Error while initializing Python pypeline";
    }




//    SERENITY_LOG(INFO) << "Importing...";
//    pySerenityPypelineModule =
//      PyImport_ImportModule(cfgSerenityPypelineModule.c_str());
//    SERENITY_LOG(INFO) << "Debug...";
//    if (pySerenityPypelineModule == NULL) {
//      PyErr_Print();
//      SERENITY_LOG(ERROR) << "Cannot import Python Module "
//      << cfgSerenityPypelineModule << " from path: "
//      << cfgSerenityPypelinePath;
//
//      return;
//    }
//
//    SERENITY_LOG(INFO) << "Imported Python Module "
//    << cfgSerenityPypelineModule << " from path: "
//    << cfgSerenityPypelinePath;
//
//    pyClass = PyObject_GetAttrString(pySerenityPypelineModule,
//                                     cfgSerenityPypelineClass.c_str());
//    if (pyClass == NULL) {
//      PyErr_Print();
//      SERENITY_LOG(ERROR) << "Cannot get class "
//      << cfgSerenityPypelineClass << " from path: "
//      << cfgSerenityPypelinePath;
//    }
//
//    pyInstance = PyObject_CallObject(pyClass, NULL);
//    if (pyInstance == NULL) {
//      PyErr_Print();
//      SERENITY_LOG(ERROR) << "Cannot instantiate "
//      << cfgSerenityPypelineClass << " from path: "
//      << cfgSerenityPypelinePath;
//
//      return;
//    }
//    Py_INCREF(pyInstance);
//
//
//
//    pyRunMethod = PyObject_GetAttrString(pyInstance, "run");
//    if (pyRunMethod == NULL) {
//      PyErr_Print();
//      SERENITY_LOG(ERROR) << "Run method undefined in class: "
//        << cfgSerenityPypelineClass;
//
//      return;
//    }
  }
};

}  // namespace serenity
}  // namespace mesos

#endif  // SERENITY_PYPELINE_FILTER_HPP
