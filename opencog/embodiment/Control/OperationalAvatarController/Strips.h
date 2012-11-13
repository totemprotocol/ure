/*
 * Strips.h
 *
 * Copyright (C) 2012 by OpenCog Foundation
 * Written by Shujing KE
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _OCPANNER_STRIPS_H
#define _OCPANNER_STRIPS_H


#include <vector>
#include <boost/variant.hpp>
#include <opencog/embodiment/Control/PerceptionActionInterface/ActionParameter.h>
#include <opencog/embodiment/Control/PerceptionActionInterface/ActionParamType.h>
#include <opencog/embodiment/Control/PerceptionActionInterface/ActionType.h>
#include <opencog/embodiment/Control/PerceptionActionInterface/PetAction.h>
#include "PlanningHeaderFiles.h"

using namespace std;
using namespace opencog::pai;
using namespace boost;

// The Actions described in STRIPS format
// Including the preconditions,effects,parameters of the action
using namespace opencog::pai;

namespace opencog { namespace oac {

class Inquery;

    enum EFFECT_OPERATOR_TYPE
    {
        OP_REVERSE, // this is only for the bool variables
        OP_ASSIGN,  // this operator can be used in any variable type =
        OP_ASSIGN_NOT_EQUAL_TO, // this operator can be used in any variable type !=
        OP_ADD,     // only for numeric variables +=
        OP_SUB,     // only for numeric variables -=
        OP_MUL,     // only for numeric variables *=
        OP_DIV,     // only for numeric variables /=
        OP_NUM_OPS  // must always be the last one in this list.
    };

    extern const char* EFFECT_OPERATOR_NAME[7];


    // There are 3 kinds of state types:
    // 1. equal, which can be used in non-numeric and numeric state values: e.g.: the price of Book1 is 15 dollors
    // 2. fuzzy value, e.g.: the price of Book1 is between 10~20 dollors
    // 3. comparator: the price of Book1 is greater than 10, less than 20
    enum StateType
    {
        STATE_EQUAL_TO,     // EvaluationLink
        STATE_NOT_EQUAL_TO,     // EvaluationLink
        STATE_FUZZY_WITHIN, // EvaluationLink + PredicationNode "Fuzzy_within"
        STATE_GREATER_THAN, // GreaterThanLink
        STATE_LESS_THAN     // LessThanLink
    };

    // some kind of state values cannot directly get from the Atomspace.see inquery.h
    // so each of the state value need a coresponding funciton to inquery the state value in real time.
    // the vector<string> is the stateOwnerList
    typedef StateValue (*InqueryFun)(const vector<StateValue>&);

    // A state is an environment variable representing some feature of the system in a certain time point
    // like the foodState of the egg(id_5904) is now RAW
    // it would be represented as an predicate link in the Atomspace
    // e.g.
    /*(AtTimeLink (stv 1 1) (av -3 0 0)
       (TimeNode "12491327572" (av -3 0 0))
       (EvaluationLink (stv 1 0.0012484394) (av -3 0 0)
          (PredicateNode "foodState" (av -3 1 0))
          (ListLink (av -3 0 0)
             (ObjectNode "id_5904" (av -3 0 0))
             (ConceptNode "RAW" (av -3 0 0))
          )
       )
    )*/
    class State
    {
    public:
        string name() const {return stateVariable->getName();}

        StateType getStateType() const {return stateType;}

        void changeStateType(StateType newType){this->stateType = newType;}

        // the state value needed real time inquery is represented as:
        // ExecutionOutputLink
        //         SchemaNode "Distance"
        bool is_need_inquery() const {return need_inquery;}

        const StateValue& getStateValue() const {return stateVariable->getValue();}

        StateValuleType getStateValuleType() const {return stateVariable->getType();}

        const vector<StateValue>& getStateOwnerList() const {return stateOwnerList;}

        InqueryFun getInqueryFun() const {return inqueryFun;}

        State(string _stateName, StateValuleType _valuetype,StateType _stateType, StateValue  _stateValue,
              vector<StateValue> _stateOwnerList, bool _need_inquery = false, InqueryFun _inqueryFun = 0);

        State(string _stateName, StateValuleType _valuetype ,StateType _stateType, StateValue _stateValue,
               bool _need_inquery = false, InqueryFun _inqueryFun = 0);
        ~State();

        void assignValue(const StateValue& newValue);


        void addOwner(StateValue& _owner)
        {
            stateOwnerList.push_back(_owner);
        }

        inline bool isSameState(const State& other) const
        {
            return ((name() == other.name())&&(stateOwnerList == other.getStateOwnerList()));
        }

        // @ satisfiedDegree is a return value between (-infinity,1.0], which shows how many percentage has this goal been achieved,
        //   if the value is getting father away from goal, it will be negative
        //   satisfiedDegree = |original - current|/|original - goal|
        //   when it's a boolean goal, only can be 0.0 or 1.0
        // @ original_state is the corresponding begin state of this goal state, so that we can compare the current state to both fo the goal and origninal states
        //                  to calculate its satisfiedDegree value.
        // when original_state is not given (defaultly 0), then no satisfiedDegree is going to be calculated
        bool isSatisfiedMe(const StateValue& value, float& satisfiedDegree, const State *original_state = 0) const;
        bool isSatisfied(const State& goal, float &satisfiedDegree, const State *original_state = 0) const;


        // To get int,float value or fuzzy int or float value from a state
        // For convenience, we will also consider int value as float value
        // if the value type is not numberic, return false and the floatVal and fuzzyFloat will not be assigned value
        bool getNumbericValues(int& intVal, float& floatVal,opencog::pai::FuzzyIntervalInt& fuzzyInt, opencog::pai::FuzzyIntervalFloat& fuzzyFloat) const;

        // About the calculation of Satisfie Degree
        // compare 3 statevalue for a same state: the original statevalue, the current statevalue and the goal statevalue
        // to see how many persentage the current value has achieved the goal, compared to the original value
        // it can be negative, when it's getting farther away from the goal than the original value
        // For these 3, the state should be the same state, but the state operator type can be different, e.g.:
        // the goal is to eat more than 10 apples, and at the beginning 2 apple has been eaten, and current has finished eaten 3 apples and begin to eat the 6th one, so:
        // original: Num_eaten(apple) = 3
        // current:  5 < Num_eaten(apple) < 6
        // goal:     Num_eaten(apple) > 10
        // so the distance between original and goal is 10 - 3 = 7
        // the distance between current and goal is ((10 - 5) +(10 - 6)) / 2 = 5.5
        // the SatifiedDegree = (7 - 5.5)/7 = 0.2143
        float static calculateNumbericsatisfiedDegree(float goal, float current, float origin);
        float static calculateNumbericsatisfiedDegree(const FuzzyIntervalFloat& goal, float current, float origin);
        float static calculateNumbericsatisfiedDegree(const FuzzyIntervalFloat& goal, const FuzzyIntervalFloat& current, const FuzzyIntervalFloat& origin);
        float static distanceBetween2FuzzyFloat(const FuzzyIntervalFloat& goal, const FuzzyIntervalFloat& other);

        inline bool operator == (State& other) const
        {
            if (name() != other.name())
                return false;

            if (!(getStateValue() == other.getStateValue()))
                return false;

            if ( !(this->stateOwnerList == other.stateOwnerList))
                return false;

            return true;

        }

    protected:

        StateVariable* stateVariable;

        // whose feature this state describes. e.g. the robot's energy
        // sometimes it's more than one ower, e.g. the relationship between RobotA and RobotB
        // typedef variant<string, Rotation, Vector, Entity > StateValue
        // the owner can be any type
        vector<StateValue> stateOwnerList;

        // see the enum StateType
        StateType stateType;

        // some kinds of state is not apparent, need real-time inquery from the system
        // e.g.: Distance(a,b) - the distance between a and b, it's usually not apparent, need to call corresponding funciton to calcuate
        // e.g.: the height of Ben, this is a essential attribute, which is usuall apparent and not changed, so this is not need a real-time inquery
        bool need_inquery;

        // if the need_inquery is true, then it needs a inquery funciton to get the value of the state in real-time
        InqueryFun inqueryFun;

    };


    class Effect
    {
    public:
        // the specific state this effect will take effect on
        State* state;

        //e.g. when this effect is to add the old stateValue by 5,
        //then effectOp = OP_ADD, opStateValue = 5
        EFFECT_OPERATOR_TYPE effectOp;
        StateValue opStateValue;

        Effect(State* _state, EFFECT_OPERATOR_TYPE _op, StateValue _OPValue);


        // only when there are misusge of the value type of opStateValue, it will return false,
        // e.g. if assign a string to a bool state, it will return false
        bool executeEffectOp();

        // make sure the value type of the operater value is the same with the value type of the state
        // and also this value type can be the parameter of this operator
        static bool _AssertValueType(State& _state, EFFECT_OPERATOR_TYPE effectOp, StateValue &OPValue);

    };


    // the float in this pair is the probability of the happenning of this Effect
    typedef pair<float,Effect*> EffectPair;

    // the rule to define the preconditions of an action and what effects it would cause
    // A rule will be represented in the Atomspace by ImplicationLink
   /*     ImplicationLink
    *         AndLink
    *             AndLink //(preconditions)
    *                 EvaluationLink
    *                     GroundedPredicateNode "precondition_1_name"
    *                     ListLink
    *                         Node:arguments
    *                         ...
    *                 EvaluationLink
    *                     PredicateNode         "precondition_2_name"
    *                     ListLink
    *                         Node:arguments
    *                         ...
    *                 ...
    *
    *             ExecutionLink //(action)
    *                 GroundedSchemaNode "schema_name"
    *                 ListLink
    *                     Node:arguments
    *                     ...
    *                 ...
    *
    *        AndLink //(effects)
    *             (SimpleTruthValue indicates the probability of the happening of an effect
    *             EvaluationLink <0.5,1.0>
    *                 GroundedPredicateNode "Effect_state_1_name"
    *                 ListLink
    *                     Node:arguments
    *                     ...
    *             EvaluationLink <0.7,1.0>
    *                 GroundedPredicateNode "Effect_stare_2_name"
    *                 ListLink
    *                     Node:arguments
    *                     ...
    */
    struct Rule
    {
    public:
        PetAction* action;

        // the avatar who carry out this action
        Entity actor;

        // the cost of this action under the conditions of this rule (the cost value is between 0 ~ 100)
        int cost;

        // All the precondition required to perform this action
        vector<State*> preconditionList;

        // All the effect this action may cause
        // there are probability for each effect
        vector<EffectPair> effectList;

        Rule(PetAction* _action, Entity _actor, int _cost, vector<State*> _preconditionList, vector<EffectPair> _effectList):
            action(_action) , actor(_actor), cost(_cost), preconditionList(_preconditionList), effectList(_effectList){}

        Rule(PetAction* _action, Entity _actor, int _cost):
            action(_action) , actor(_actor), cost(_cost){}

        void addEffect(EffectPair effect)
        {
            effectList.push_back(effect);
        }

        void addPrecondition(State* precondition)
        {
            preconditionList.push_back(precondition);
        }

    };


}}


#endif
