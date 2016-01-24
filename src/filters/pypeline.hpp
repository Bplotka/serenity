#ifndef SERENITY_PYPELINE_FILTER_HPP
#define SERENITY_PYPELINE_FILTER_HPP

#include <ctime>
#include <list>
#include <memory>
#include <string>

#include <boost/python.hpp>

#include "mesos/mesos.hpp"
#include "google/protobuf/message.h"

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
    this->fields[python_pipeline::SERENITY_PYPELINE_RUN] =
      (std::string) python_pipeline::DEFAULT_SERENITY_PYPELINE_RUN;
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

    cfgSerenityPypelineRun =
      config.getS(python_pipeline::SERENITY_PYPELINE_RUN);

    initializePythonInterpreter();
  }

  ~PypelineFilter() {
  }

  static const constexpr char* NAME = "PypelineFilter";

  Try<Nothing> consume(const ResourceUsage& in) {
    if (pyInstance.is_none()) {
      return Error("Class from path " + cfgSerenityPypelinePath
                   + " not initialized.");
    }

    Contentions contentions = Contentions();

    try {
      // TODO(bplotka): Pass Usage things inside.
      std::string usageStr;
      in.SerializeToString(&usageStr);

      boost::python::str usageProto(usageStr);

      pyResponse =
        pyInstance.attr(cfgSerenityPypelineRun.c_str())(usageProto);

    } catch (const std::exception e) {
      PyErr_Print();
      PyErr_Clear();
      SERENITY_LOG(ERROR) << "Error while initializing Pypeline";
    }

    // Continue pipeline. Currently, we won't receive any automatic
    // scheduling actions.
    produce(contentions);

    return Nothing();
  }

 private:
  const Tag tag;

  boost::python::object pyModule, pyInstance, pyResponse;

  std::string cfgSerenityPypelinePath;
  std::string cfgSerenityPypelineModule;
  std::string cfgSerenityPypelineClass;
  std::string cfgSerenityPypelineRun;

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

    } catch (const std::exception e) {
      PyErr_Print();
      PyErr_Clear();
      SERENITY_LOG(ERROR) << "Error while initializing Pypeline";
    }

    SERENITY_LOG(INFO) << "Pypeline initialized.";
  }

};

}  // namespace serenity
}  // namespace mesos

#endif  // SERENITY_PYPELINE_FILTER_HPP
