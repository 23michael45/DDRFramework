//
//  state_machine.h
//

#ifndef state_machine_h
#define state_machine_h

#include <unordered_map>
#include <memory>
#include "../Utility/DDRMacro.h"
#include "../Utility/Logger.h"

template <class PT>
class StateMachine;

#define ADD_STATE(sm,st) std::shared_ptr<st> sp##st = std::make_shared<st>(shared_from_this());\
	sm->addState<st>(sp##st);\
	sp##st->SetStateMachine(sm);


template <class T>
class State
{
public:
	State(std::shared_ptr<T> sp):m_spParentObject(sp)
	{
	}
	~State()
	{
		m_spParentObject.reset();
		m_spParentStateMachine.reset();
	}
	/**
	 *  Called when entering state
	 *
	 *  @param previousState previous state or null if this is the first state
	 */
	virtual void didEnterWithPreviousState(std::shared_ptr<State<T>> previousState) {};
	/**
	 *  Called every frame by state machine
	 *
	 *  @param delta time
	 */
	virtual void updateWithDeltaTime(float delta) {};
	/**
	 *  Checks if next state is valid for transition
	 *
	 *  @param state next state
	 *
	 *  @return true if valid, false otherwise
	 */
	virtual bool isValidNextState(std::shared_ptr<State<T>> state) { return false; };
	/**
	 *  Called when exiting current state
	 *
	 *  @param nextState next state
	 */
	virtual void willExitWithNextState(std::shared_ptr<State<T>> nextState) {};

	void SetStateMachine(std::shared_ptr<StateMachine<T>> sp)
	{
		m_spParentStateMachine = sp;
	}

protected:
	std::weak_ptr<StateMachine<T>> m_spParentStateMachine;
	std::weak_ptr<T> m_spParentObject;
};

template <class PT>
class StateMachine
{
public:

	StateMachine()
	{

	}
	/**
	 *  Find state by id and return casted state
	 *
	 *  @return casted state
	 */
	template<class T>
	std::shared_ptr<T> findState()
	{
		const std::string name = typeid(T).name();
		if (m_States.count(name) != 0)
		{
			return std::dynamic_pointer_cast<T>(m_States[name]);
		}
		return nullptr;
	}
	std::shared_ptr<State<PT>> findStateByName(std::string name)
	{
		if (m_States.count(name) != 0)
		{
			return  m_States[name];
		}
		return nullptr;
	}

	/**
	 *  Add new state to state machine
	 *
	 *  @param args arguments to pass to constructor of state
	 */
	template<typename T, class... Args>
	void addState(Args&&... args)
	{
		auto typeId = typeid(T).name();

		m_States.insert({ typeId, std::forward<Args>(args)... });
	}

	/**
	 *  Check if we can enter state
	 *
	 *  @return true if this state is valid, false otherwise
	 */
	template<typename T>
	bool canEnterState()
	{
		if (m_spState == nullptr)
		{
			return true;
		}
		else
		{
			auto state = findState<T>();
			if (state)
			{
				return m_spState->isValidNextState(state);
			}
		}
		return false;
	}

	/**
	 *  Enters new state
	 *
	 *  Before entering new state old state will check if it is a valid state to execute
	 *  transaction
	 *
	 *  Order of execution:
	 *
	 *  willExitWithNextState will be called on current state
	 *  didEnterWithPreviousState will be called on new state
	 *
	 *  @return true if entered, false otherwise
	 */
	template<class T>
	bool enterState()
	{
		auto state = findState<T>();
		if (state)
		{
			if (m_spState == nullptr)
			{
				m_spState = state;
				m_spState->didEnterWithPreviousState(nullptr);
				return true;
			}
			else
			{
				if (m_spState->isValidNextState(state))
				{
					m_spState->willExitWithNextState(state);
					state->didEnterWithPreviousState(m_spState);
					m_spState = state;
					return true;
				}
			}
		}
		return false;
	}
	bool enterState(std::string name)
	{
		if (m_States.count(name) != 0)
		{
			auto state =  m_States[name];
			if (state)
			{
				if (m_spState == nullptr)
				{
					m_spState = state;
					m_spState->didEnterWithPreviousState(nullptr);
					return true;
				}
				else
				{
					if (m_spState->isValidNextState(state))
					{
						m_spState->willExitWithNextState(state);
						state->didEnterWithPreviousState(m_spState);
						m_spState = state;
						return true;
					}
				}
			}
			return false;
		}

	}

	/**
	 *  Enters new state without any check if next state is valid
	 *
	 *
	 *  Order of execution:
	 *
	 *  willExitWithNextState will be called on current state
	 *  didEnterWithPreviousState will be called on new state
	 *
	 *  @return true if entered, false otherwise
	 */
	bool setState()
	{
		auto state = findState<std::shared_ptr<State<PT>>>();
		if (state)
		{
			if (m_spState == nullptr)
			{
				m_spState = state;
				m_spState->didEnterWithPreviousState(nullptr);
				return true;
			}
			else
			{
				m_spState->willExitWithNextState(state);
				m_spState->didEnterWithPreviousState(m_spState);
				m_spState = state;
				return true;
			}
		}
		return false;
	}

	/**
	 *  Update state machine delta time, this will call updateWithDeltaTime on current state
	 *
	 *  @param delta delta time
	 */
	void updateWithDeltaTime(float delta = 0)
	{
		if (m_spState != nullptr)
		{
			m_spState->updateWithDeltaTime(delta);
		}
	}

	/**
	 *  Get current state
	 *
	 *  @return current state
	 */
	State<PT>* getState()
	{
		return m_spState.get();
	}

	~StateMachine()
	{
		DebugLog("\nStateMachine Destroy");

		for (auto iter = m_States.begin(); iter != m_States.end(); ++iter)
		{
			std::shared_ptr<State<PT>> sp = iter->second;
			sp.reset();
		}

	}

private:
	std::unordered_map<std::string, std::shared_ptr<State<PT>>> m_States;
	std::shared_ptr<State<PT>> m_spState;

};

#endif /* state_machine_h */