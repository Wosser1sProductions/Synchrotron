/**
*	Synchronous component with combined Signals and Slots functionality.
* 		Loosely based on https://github.com/winglot/Signals
*/
#ifndef SYNCHROTRONCOMPONENTFList_HPP
#define SYNCHROTRONCOMPONENTFList_HPP

#include <iostream> // For testing for now

#include "SynchrotronComponent.hpp"
#include <bitset>
#include <forward_list>

namespace Synchrotron {

	/** \brief
	 *	SynchrotronComponent is the base for all components,
	 *	offering in and output connections to other SynchrotronComponent.
	 *
	 *	\param	bit_width
	 *		This template argument specifies the width of the internal bitset state.
	 */
	template <size_t bit_width>
	class SynchrotronComponentFList : public Mutex {
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
			std::forward_list<SynchrotronComponentFList*> slotOutput;

			/**	\brief
			 *	**Signals == inputs**
			 *
			 *		Receive tick()s from these subscriptions in signalInput.
			 */
			std::forward_list<SynchrotronComponentFList*> signalInput;

			/**	\brief	Connect a new slot s:
			 *		* Add s to this SynchrotronComponentFList's outputs.
			 *		* Add this to s's inputs.
			 *
			 *	\param	s
			 *		The SynchrotronComponentFList to connect.
			 */
			inline void connectSlot(SynchrotronComponentFList* s) {
				//LockBlock lock(this);

				this->slotOutput.push_front(s);
				s->signalInput.push_front(this);
			}

			/**	\brief	Disconnect a slot s:
			 *		* Remove s from this SynchrotronComponentFList's outputs.
			 *		* Remove this from s's inputs.
			 *
			 *	\param	s
			 *		The SynchrotronComponentFList to disconnect.
			 */
			inline void disconnectSlot(SynchrotronComponentFList* s) {
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
			SynchrotronComponentFList(size_t initial_value = 0) : state(initial_value) {}

			/**	\brief
			 *	Copy constructor
			 *	*	Duplicates signal subscriptions (inputs)
			 *	*	Optionally also duplicates slot connections (outputs)
			 *
			 *	\param	sc const
			 *		The other SynchrotronComponentFList to duplicate the connections from.
			 *	\param	duplicateAll_IO
			 *		Specifies whether to only copy inputs (false) or outputs as well (true).
			 */
			SynchrotronComponentFList(const SynchrotronComponentFList& sc, bool duplicateAll_IO = false) : SynchrotronComponentFList() {
				//LockBlock lock(this);

				// Copy subscriptions
				for(auto& sender : sc.signalInput) {
					this->addInput(*sender);
				}

				if (duplicateAll_IO) {
					// Copy subscribers
					for(auto& connection : sc.slotOutput) {
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
			SynchrotronComponentFList(std::initializer_list<SynchrotronComponentFList*> inputList,
								 std::initializer_list<SynchrotronComponentFList*> outputList = {})
									: SynchrotronComponentFList() {
				this->addInput(inputList);
				this->addOutput(outputList);
			}

			/** \brief	Default destructor
			 *
			 *		When called, will disconnect all in and output connections to this SynchrotronComponentFList.
			 */
			~SynchrotronComponentFList() {
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

			/**	\brief	Gets this SynchrotronComponentFList's bit width.
			 *
			 *	\return	size_t
			 *      Returns the bit width of the internal bitset.
			 */
			size_t getBitWidth() const {
				return bit_width;
			}

//			/* No real use since function cannot be called with different size SynchrotronComponentFLists */
//			/* Maybe viable when SynchrotronComponentFList has different in and output sizes */
//			/*	\brief	Compare this bit width to that of other.
//             *
//             *	\param	other
//			 *		The other SynchrotronComponentFList to check.
//			 *
//             *	\return	bool
//             *      Returns whether the widths match.
//             */
//			inline bool hasSameWidth(SynchrotronComponentFList& other) {
//				return this->getBitWidth() == other.getBitWidth();
//			}

			/**	\brief	Gets this SynchrotronComponentFList's state.
			 *
			 *	\return	std::bitset<bit_width>
			 *      Returns the internal bitset.
			 */
			inline std::bitset<bit_width> getState() const {
				return this->state;
			}

			/**	\brief	Gets the SynchrotronComponentFList's input connections.
			 *
			 *	\return	std::set<SynchrotronComponentFList*>&
			 *      Returns a reference set to this SynchrotronComponentFList's inputs.
			 */
			const std::forward_list<SynchrotronComponentFList*>& getIputs() const {
				return this->signalInput;
			}

			/**	\brief	Gets the SynchrotronComponentFList's output connections.
			 *
			 *	\return	std::set<SynchrotronComponentFList*>&
			 *      Returns a reference set to this SynchrotronComponentFList's outputs.
			 */
			const std::forward_list<SynchrotronComponentFList*>& getOutputs() const {
				return this->slotOutput;
			}

			/**	\brief	Adds/Connects a new input to this SynchrotronComponentFList.
			 *
			 *	**Ensures both way connection will be made:**
			 *	This will have input added to its inputs and input will have this added to its outputs.
			 *
			 *	\param	input
			 *		The SynchrotronComponentFList to connect as input.
			 */
			void addInput(SynchrotronComponentFList& input) {
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
			void addInput(std::initializer_list<SynchrotronComponentFList*> inputList) {
				for(auto connection : inputList)
					this->addInput(*connection);
			}

			/**	\brief	Removes/Disconnects an input to this SynchrotronComponentFList.
			 *
			 *	**Ensures both way connection will be removed:**
			 *	This will have input removed from its inputs and input will have this removed from its outputs.
			 *
			 *	\param	input
			 *		The SynchrotronComponentFList to disconnect as input.
			 */
			void removeInput(SynchrotronComponentFList& input) {
				LockBlock lock(this);

				input.disconnectSlot(this);
			}

			/**	\brief	Adds/Connects a new output to this SynchrotronComponentFList.
			 *
			 *	**Ensures both way connection will be made:**
			 *	This will have output added to its outputs and output will have this added to its inputs.
			 *
			 *	\param	output
			 *		The SynchrotronComponentFList to connect as output.
			 */
			void addOutput(SynchrotronComponentFList& output) {
				LockBlock lock(this);

				// deprecated? //if (!this->hasSameWidth(*output)) return false;
				this->connectSlot(&output);
			}

			/**	\brief	Adds/Connects a list of new outputs to this SynchrotronComponent.
			 *
			 *	Calls addOutput() on each SynchrotronComponent* in outputList.
			 *
			 *	\param	outputList
			 *		The list of SynchrotronComponents to connect as output.
			 */
			void addOutput(std::initializer_list<SynchrotronComponentFList*> outputList) {
				for(auto connection : outputList)
					this->addOutput(*connection);
			}

			/**	\brief	Removes/Disconnects an output to this SynchrotronComponentFList.
			 *
			 *	**Ensures both way connection will be removed:**
			 *	This will have output removed from its output and output will have this removed from its inputs.
			 *
			 *	\param	output
			 *		The SynchrotronComponentFList to disconnect as output.
			 */
			void removeOutput(SynchrotronComponentFList& output) {
				LockBlock lock(this);

				this->disconnectSlot(&output);
			}

			/**	\brief	The tick() method will be called when one of this SynchrotronComponentFList's inputs issues an emit().
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
					this->state |= ((SynchrotronComponentFList*) connection)->getState();
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


#endif // SynchrotronComponentFListFList_HPP
