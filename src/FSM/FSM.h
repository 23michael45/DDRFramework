/*!
 * File: FSM.h
 * Date: 2019/04/29 15:51
 *
 * Author: michael
 * Contact: michael2345@live.cn
 *
 * Description:Finite State Machine Template Class
 *
*/
#ifndef state_machine_h
#define state_machine_h

#include <unordered_map>
#include <memory>
#include <src/Utility/DDRMacro.h>

template <class PT>
class StateMachine;

#define ADD_STATE(sm,st) std::shared_ptr<st> sp##st = std::make_shared<st>(shared_from_this());\
sm->addState<st>(sp##st);\
sp##st->SetStateMachine(sm);

#define ADD_SUBSTATE(sm,st,spentity) std::shared_ptr<st> sp##st = std::make_shared<st>(spentity,this);\
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
	virtual void updateWithDeltaTime(double deltaTime) {};
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

	~StateMachine()
	{
		DebugLog("StateMachine Destroy");

		for (auto iter = m_States.begin(); iter != m_States.end(); ++iter)
		{
			std::shared_ptr<State<PT>> sp = iter->second;
			sp.reset();
		}
		m_States.clear();

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

	template<class T>
	bool isState()
	{
		auto sp = std::dynamic_pointer_cast<T>(m_spState);
		return sp != nullptr;
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

			if (m_spState)
			{
				if (m_spState->isValidNextState(state))
				{
					m_spNextState = state;
					return true;
				}
			}
			else
			{

				m_spNextState = state;
				return true;
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
				if (m_spState)
				{
					if (m_spState->isValidNextState(state))
					{
						m_spNextState = state;
						return true;
					}
				}
				else
				{
					m_spNextState = state;
					return true;
				}
			}
		}


		return false;
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
	void updateWithDeltaTime(double deltaTime = 0)
	{
		//do not lock here,enterstate will lock in updateWithDeltaTime
		//m_StateMutex.lock(); 

		try
		{
			if (m_spNextState)
			{
				if (m_spState)
				{
					m_spState->willExitWithNextState(m_spNextState);
				}

				m_spNextState->didEnterWithPreviousState(m_spState);
				m_spState = m_spNextState;
				m_spNextState = nullptr;


			}

			if (m_spState)
			{
				m_spState->updateWithDeltaTime(deltaTime);

			}
		}
		catch (std::exception& e)
		{
			DebugLog("\nupdateWithDeltaTime Error", e.what());

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


private:
	std::unordered_map<std::string, std::shared_ptr<State<PT>>> m_States;
	std::shared_ptr<State<PT>> m_spState;
	std::shared_ptr<State<PT>> m_spNextState;
};

#endif /* state_machine_h */