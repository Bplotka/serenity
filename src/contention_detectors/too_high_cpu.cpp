#include <string>
#include <utility>

#include "contention_detectors/too_high_cpu.hpp"

#include "mesos/resources.hpp"

namespace mesos {
namespace serenity {

Try<Nothing> TooHighCpuUsageDetector::consume(const ResourceUsage& in) {
  Contentions product;

  if (in.total_size() == 0) {
    return Error(std::string(NAME) + " No total in ResourceUsage");
  }

  Resources totalAgentResources(in.total());
  Option<double_t> totalAgentCpus = totalAgentResources.cpus();

  if (totalAgentCpus.isNone()) {
    return Error(std::string(NAME) + " No total cpus in ResourceUsage");
  }

  double_t agentSumValue = 0;
  uint64_t beExecutors = 0;

  for (const ResourceUsage_Executor& inExec : in.executors()) {
    if (!inExec.has_executor_info()) {
      SERENITY_LOG(ERROR) << "Executor <unknown>"
      << " does not include executor_info";
      // Filter out these executors.
      continue;
    }
    if (!inExec.has_statistics()) {
      SERENITY_LOG(ERROR) << "Executor "
      << inExec.executor_info().executor_id().value()
      << " does not include statistics.";
      // Filter out these executors.
      continue;
    }

    Try<double_t> value = this->cpuUsageGetFunction(inExec);
    if (value.isError()) {
      SERENITY_LOG(ERROR) << value.error();
      continue;
    }

    agentSumValue += value.get();

    if (!Resources(inExec.allocated()).revocable().empty()) {
      beExecutors++;
    }
  }

  // Debug only
  SERENITY_LOG(INFO) << "Sum = " << agentSumValue << " vs total = "
                     << totalAgentCpus.get();
  double_t lvl = agentSumValue / totalAgentCpus.get();

  if (lvl > this->cfgUtilizationThreshold) {
    if (beExecutors == 0) {
      SERENITY_LOG(INFO) << "No BE tasks - only high host utilization";
    } else {
      SERENITY_LOG(INFO) << "Creating CPU contention, because of the value"
                         << " above the threshold. " << agentSumValue << "/"
                         << totalAgentCpus.get();
      product.push_back(createContention(totalAgentCpus.get() - agentSumValue,
                                         Contention_Type_CPU));
    }
  }

  // Continue pipeline.
  this->produce(product);

  return Nothing();
}


}  // namespace serenity
}  // namespace mesos
