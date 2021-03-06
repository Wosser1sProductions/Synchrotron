/**
*	Synchronous component with combined Signals and Slots functionality.
* 		Loosely based on https://github.com/winglot/Signals
*/
#ifndef SYNCHROTRONCOMPONENTList_HPP
#define SYNCHROTRONCOMPONENTList_HPP

#include <iostream> // For testing for now

#include "SynchrotronComponent.hpp"
#include <bitset>
#include <list>

namespace Synchrotron {

	/** \brief
	 *	SynchrotronComponent is the base for all components,
	 *	offering in and output connections to other SynchrotronComponent.
	 *
	 *	\param	bit_width
	 *		This template argument specifies the width of the internal bitset state.
     */
	template <size_t bit_width>
	class SynchrotronComponentList : public Mutex {
		private:
			/**	\brief
			 *	The current internal state of bits in this component (default output).
			 */
			std::bitset<bit_width> state;

			/**	\brief
			 *	**Slots == outputs**
			 *
			 *		Emit this.signal to subscribers in slotOutput.
			 */
			std::list<SynchrotronComponentList*> slotOutput;

			/**	\brief
			 *	**Signals == inputs**
			 *
			 *		Receive tick()s from these subscriptions in signalInput.
			 */
			std::list<SynchrotronComponentList*> signalInput;

            /**	\brief	Connect a new slot s:
             *		* Add s to this SynchrotronComponent's outputs.
             *		* Add this to s's inputs.
             *
             *	\param	s
			 *		The SynchrotronComponent to connect.
             */
			inline void connectSlot(SynchrotronComponentList* s) {
				//LockBlock lock(this);

				this->slotOutput.push_back(s);
				s->signalInput.push_back(this);
			}

            /**	\brief	Disconnect a slot s:
             *		* Remove s from this SynchrotronComponent's outputs.
             *		* Remove this from s's inputs.
             *
             *	\param	s
			 *		The SynchrotronComponent to disconnect.
             */
			inline void disconnectSlot(SynchrotronComponentList* s) {
				//LockBlock lock(this);

				this->slotOutput.remove(s);
				s->signalInput.remove(this);
			}

		public:
            /** \brief	Default constructor
             *
             *	\param	initial_value
			 *		The initial state of the internal bitset.
			 *	\param	bit_width
			 *		The size of the internal width of the bitset.
             */
			SynchrotronComponentList(size_t initial_value = 0) : state(initial_value) {}

			/**	\brief
			 *	Copy constructor
			 *	*	Duplicates signal subscriptions (inputs)
			 *	*	Optionally also duplicates slot connections (outputs)
			 *
			 *	\param	sc const
			 *		The other SynchrotronComponent to duplicate the connections from.
			 *	\param	duplicateAll_IO
			 *		Specifies whether to only copy inputs (false) or outputs as well (true).
			 */
			SynchrotronComponentList(const SynchrotronComponentList& sc, bool duplicateAll_IO = false) : SynchrotronComponentList() {
				//LockBlock lock(this);

				// Copy subscriptions
				for(auto& sender : sc.getIputs()) {
					this->addInput(*sender);
				}

				if (duplicateAll_IO) {
					// Copy subscribers
					for(auto& connection : sc.getOutputs()) {
						this->addOutput(*connection);
					}
				}
			}

			/**	\brief
			 *	Connection constructor
			 *	*	Adds signal subscriptions from inputList
			 *	*	Optionally adds slot subscribers from outputList
			 *
			 *	\param	inputList
			 *		The list of SynchrotronComponents to connect as input.
			 *	\param	outputList
			 *		The list of SynchrotronComponents to connect as output..
			 */
			SynchrotronComponentList(std::initializer_list<SynchrotronComponentList*> inputList,
								 std::initializer_list<SynchrotronComponentList*> outputList = {})
									: SynchrotronComponentList() {
				this->addInput(inputList);
				this->addOutput(outputList);
			}

			/** \brief	Default destructor
			 *
			 *		When called, will disconnect all in and output connections to this SynchrotronComponent.
             */
			~SynchrotronComponentList() {
				LockBlock lock(this);

				// Disconnect all Slots
				for(auto& connection : this->slotOutput) {
					connection->signalInput.remove(this);
					//delete connection; //?
				}

				// Disconnect all Signals
				for(auto &sender: this->signalInput) {
					sender->slotOutput.remove(this);
				}

				this->slotOutput.clear();
				this->signalInput.clear();
			}

            /**	\brief	Gets this SynchrotronComponent's bit width.
             *
             *	\return	size_t
             *      Returns the bit width of the internal bitset.
             */
			size_t getBitWidth() const {
				return bit_width;
			}

//			/* No real use since function cannot be called with different size SynchrotronComponents */
//			/* Maybe viable when SynchrotronComponent has different in and output sizes */
//			/*	\brief	Compare this bit width to that of other.
//             *
//             *	\param	other
//			 *		The other SynchrotronComponent to check.
//			 *
//             *	\return	bool
//             *      Returns whether the widths match.
//             */
//			inline bool hasSameWidth(SynchrotronComponent& other) {
//				return this->getBitWidth() == other.getBitWidth();
//			}

			/**	\brief	Gets this SynchrotronComponent's state.
             *
             *	\return	std::bitset<bit_width>
             *      Returns the internal bitset.
             */
			inline std::bitset<bit_width> getState() const {
				return this->state;
			}

			/**	\brief	Gets the SynchrotronComponent's input connections.
             *
             *	\return	std::set<SynchrotronComponent*>&
             *      Returns a reference set to this SynchrotronComponent's inputs.
             */
			const std::list<SynchrotronComponentList*>& getIputs() const {
				return this->signalInput;
			}

			/**	\brief	Gets the SynchrotronComponent's output connections.
             *
             *	\return	std::set<SynchrotronComponent*>&
             *      Returns a reference set to this SynchrotronComponent's outputs.
             */
			const std::list<SynchrotronComponentList*>& getOutputs() const {
				return this->slotOutput;
			}

            /**	\brief	Adds/Connects a new input to this SynchrotronComponent.
             *
             *	**Ensures both way connection will be made:**
             *	This will have input added to its inputs and input will have this added to its outputs.
             *
             *	\param	input
             *		The SynchrotronComponent to connect as input.
             */
			void addInput(SynchrotronComponentList& input) {
				LockBlock lock(this);

				// deprecated? //if (!this->hasSameWidth(input)) return false;
				input.connectSlot(this);
			}

			/**	\brief	Adds/Connects a list of new inputs to this SynchrotronComponent.
			 *
			 *	Calls addInput() on each SynchrotronComponent* in inputList.
			 *
			 *	\param	inputList
			 *		The list of SynchrotronComponents to connect as input.
			 */
			void addInput(std::initializer_list<SynchrotronComponentList*> inputList) {
				for(auto connection : inputList)
					this->addInput(*connection);
			}

            /**	\brief	Removes/Disconnects an input to this SynchrotronComponent.
             *
             *	**Ensures both way connection will be removed:**
             *	This will have input removed from its inputs and input will have this removed from its outputs.
             *
             *	\param	input
             *		The SynchrotronComponent to disconnect as input.
             */
			void removeInput(SynchrotronComponentList& input) {
				LockBlock lock(this);

				input.disconnectSlot(this);
			}

			/**	\brief	Adds/Connects a new output to this SynchrotronComponent.
             *
             *	**Ensures both way connection will be made:**
             *	This will have output added to its outputs and output will have this added to its inputs.
             *
             *	\param	output
             *		The SynchrotronComponent to connect as output.
             */
			void addOutput(SynchrotronComponentList& output) {
				LockBlock lock(this);

				// deprecated? //if (!this->hasSameWidth(*output)) return false;
				this->connectSlot(&output);
			}

			void addOutput(std::initializer_list<SynchrotronComponentList*> outputList) {
				for(auto connection : outputList)
					this->addOutput(*connection);
			}

			/**	\brief	Removes/Disconnects an output to this SynchrotronComponent.
             *
             *	**Ensures both way connection will be removed:**
             *	This will have output removed from its output and output will have this removed from its inputs.
             *
             *	\param	output
             *		The SynchrotronComponent to disconnect as output.
             */
			void removeOutput(SynchrotronComponentList& output) {
				LockBlock lock(this);

				this->disconnectSlot(&output);
			}

			/**	\brief	The tick() method will be called when one of this SynchrotronComponent's inputs issues an emit().
			 *
             *	\return	virtual void
             *		This method should be implemented by a derived class.
             */
			virtual void tick() {
				//LockBlock lock(this);
				std::bitset<bit_width> prevState = this->state;

				//std::cout << "Ticked\n";
				for(auto& connection : this->signalInput) {
					// Change this line to change the logic applied on the states:
					this->state |= ((SynchrotronComponentList*) connection)->getState();
				}

				// Directly emit changes to subscribers on change
				if (prevState != this->state)
					this->emit();
			}

			/**	\brief	The emit() method will be called after a tick() completes to ensure the flow of new data.
			 *
			 *	Loops over all outputs and calls tick().
			 *
             *	\return	virtual void
             *		This method can be re-implemented by a derived class.
             */
			virtual inline void emit() {
				//LockBlock lock(this);

				for(auto& connection : this->slotOutput) {
					connection->tick();
				}
				//std::cout << "Emitted\n";
			}
	};

}


#endif // SYNCHROTRONCOMPONENTList_HPP
