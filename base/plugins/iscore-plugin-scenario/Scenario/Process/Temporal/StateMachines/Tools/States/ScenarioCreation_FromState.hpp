#pragma once
#include "ScenarioCreationState.hpp"
class ScenarioCreation_FromState final : public ScenarioCreationState
{
    public:
        ScenarioCreation_FromState(
                const ScenarioStateMachine& stateMachine,
                const Path<ScenarioModel>& scenarioPath,
                iscore::CommandStack& stack,
                QState* parent);

    private:
        void createToNothing();
        void createToTimeNode();
        void createToEvent();
        void createToState();

        template<typename Fun>
        void creationCheck(Fun&& fun);
};