#pragma once
#include <Scenario/Document/Interval/IntervalModel.hpp>
#include <iscore/document/DocumentContext.hpp>
#include <iscore/tools/IdentifierGeneration.hpp>

namespace Scenario
{

template <typename Component_T>
class IntervalComponent : public Component_T
{
public:
  template <typename... Args>
  IntervalComponent(Scenario::IntervalModel& cst, Args&&... args)
      : Component_T{std::forward<Args>(args)...}, m_interval{cst}
  {
  }

  Scenario::IntervalModel& interval() const
  {
    return m_interval;
  }

  auto& context() const { return this->system().context(); }

  template <typename Models>
  auto& models() const
  {
    static_assert(
        std::is_same<Models, Process::ProcessModel>::value,
        "Interval component must be passed Process::ProcessModel as child.");

    return m_interval.processes;
  }

private:
  Scenario::IntervalModel& m_interval;
};

template <typename System_T>
using GenericIntervalComponent
    = Scenario::IntervalComponent<iscore::GenericComponent<System_T>>;
}