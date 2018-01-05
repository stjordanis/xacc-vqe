/***********************************************************************************
 * Copyright (c) 2016, UT-Battelle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the xacc nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributors:
 *   Initial API and implementation - Alex McCaskey
 *
 **********************************************************************************/
#ifndef VQE_IR_SPININSTRUCTION_HPP_
#define VQE_IR_SPININSTRUCTION_HPP_

#include "Instruction.hpp"
#include "CompositeSpinInstruction.hpp"
#include "CommonPauliProducts.hpp"
#include <map>
#include <unsupported/Eigen/KroneckerProduct>
#include "XACC.hpp"

namespace xacc {

namespace vqe {

/**
 * The SpinInstruction is a realization of the XACC Instruction
 * interface that models a single term in a spin-based
 * Hamiltonian - a term of Pauli products. At its
 * core it keeps track of a vector if int-string pairs
 * that model the qubit operated on and the Pauli gate name,
 * e.g. (0, Z) is a Z0 gate. It exposes overloaded
 * operators to make composing complex algebraic
 * composites of SpinInstructions is easier.
 *
 * Also, all SpinInstructions keep track of
 * a complex<double> coefficient.
 *
 */
class SpinInstruction: public Instruction {

protected:

	/**
	 * The Pauli gates this SpinInstruction models.
	 */
//	std::vector<std::pair<int, std::string>> terms;

	std::map<int, std::string> terms;

	/**
	 * A convenience mapping for common
	 * Pauli matrix products.
	 */
	CommonPauliProducts pauliProducts;

public:

	/**
	 * The coefficient multiplying this SpinInstruction.
	 */
	std::complex<double> coefficient = std::complex<double>(1,0);

	std::string variable = "";

	SpinInstruction() {
	}

	/**
	 * The copy constructor
	 * @param i
	 */
	SpinInstruction(const SpinInstruction& i) :
			pauliProducts(i.pauliProducts), terms(i.terms), coefficient(
					i.coefficient), variable(i.variable) {
	}

	/**
	 * The Constructor, takes a vector of
	 * qubit-gatename pairs. Initializes coefficient to 1
	 *
	 * @param operators The pauli operators making up this SpinInstruction
	 */
	SpinInstruction(std::map<int, std::string> operators) :
			terms(operators), coefficient(std::complex<double>(1, 0)) {
	}

	SpinInstruction(std::map<int, std::string> operators, std::string var) :
			terms(operators), coefficient(std::complex<double>(1, 0)), variable(var) {
	}

	/**
	 * The Constructor, takes a vector of
	 * qubit-gatename pairs and this instruction's coefficient
	 *
	 * @param operators
	 * @param coeff
	 */
	SpinInstruction(std::map<int, std::string> operators,
			std::complex<double> coeff) :
			terms(operators), coefficient(coeff) {
	}

	SpinInstruction(std::map<int, std::string> operators,
			std::complex<double> coeff, std::string var) :
			terms(operators), coefficient(coeff), variable(var) {
	}

	/**
	 * Return the name of this Instruction
	 *
	 * @return name The name of this Instruction
	 */
	virtual const std::string getName() {
		if (isIdentity()) {
			return variable+"I";
		} else {
			std::stringstream s;
			s << variable;
			for (auto& t : terms) {
				if (t.second != "I")
					s << t.second << t.first;
			}
			return s.str();
		}
	}

	const int countTermsOfType(const std::string& gate) {
		int count = 0;
		for (auto& t : terms) {
			if (t.second == gate) {
				count++;
			}
		}

		return count;
	}

	bool isIdentity() const {
		return terms.empty();
	}

	bool isDiagonal() {
		return countTermsOfType("Z") == terms.size();
	}

	const std::pair<std::string, std::complex<double>> computeActionOnKet(
			const std::string& bitString);

	const std::pair<std::string, std::complex<double>> computeActionOnBra(
			const std::string& bitString);

	virtual const std::string toString() {
		return toString("");
	}

	/**
	 * Persist this Instruction to an assembly-like
	 * string.
	 *
	 * @param bufferVarName The name of the AcceleratorBuffer
	 * @return str The assembly-like string.
	 */
	virtual const std::string toString(const std::string& bufferVarName) {
		std::stringstream ss;
		ss << coefficient << " ";
		if (!variable.empty()) {
			ss << variable << " ";
		}

		if (isIdentity()) {
			ss << "I";
			return ss.str();
		} else {
			for (auto& t : terms) {
				ss << t.second << t.first << " ";
			}
			auto r = ss.str().substr(0, ss.str().size() - 1);
			boost::trim(r);
			return r;
		}
	}

	Eigen::SparseMatrix<std::complex<double>> toSparseMatrix(
			const int nQubits);
	std::vector<int> toBinaryVector(const int nQubits) const;
	void fromBinaryVector(std::vector<int> vec,
			std::complex<double> coeff);
	Eigen::SparseMatrix<double> toSparseRealMatrix(
			const int nQubits);
	Eigen::MatrixXcd toMatrix(const int nQubits);

	/**
	 * Return the indices of the bits that this Instruction
	 * operates on.
	 *
	 * @return bits The bits this Instruction operates on.
	 */
	virtual const std::vector<int> bits() {
		std::vector<int> qbits;
		for (auto& t : terms) {
			qbits.push_back(t.first);
		}
		return qbits;
	}

	/**
	 * Return this Instruction's parameter at the given index.
	 *
	 * @param idx The index of the parameter.
	 * @return param The InstructionParameter at the given index.
	 */
	virtual InstructionParameter getParameter(const int idx) const {
		if (idx != 0) {
			XACCError("SpinInstruction keeps track of 1 parameter, the "
					"term coefficient, its Instruction Parameter "
					"index is 0. " + std::to_string(idx) + " is "
							"an invalid index.");
		}
		return InstructionParameter(coefficient);
	}

	/**
	 * Return all of this Instruction's parameters.
	 *
	 * @return params This instructions parameters.
	 */
	virtual std::vector<InstructionParameter> getParameters() {
		return std::vector<InstructionParameter>{InstructionParameter(coefficient)};
	}


	/**
	 * Set this Instruction's parameter at the given index.
	 *
	 * @param idx The index of the parameter
	 * @param inst The instruction.
	 */
	virtual void setParameter(const int idx, InstructionParameter& inst) {
		if (idx != 0) {
			XACCError("SpinInstruction keeps track of 1 parameter, the "
					"term coefficient, its Instruction Parmater "
					"index is 0. " + std::to_string(idx) + " is "
							"an invalid index.");
		}
		coefficient = boost::get<std::complex<double>>(inst);
	}

	/**
	 * Return the number of InstructionParameters this Instruction contains.
	 *
	 * @return nInsts The number of instructions.
	 */
	virtual const int nParameters() {
		return 1;
	}

	/**
	 * Return true if this Instruction is parameterized.
	 *
	 * @return parameterized True if this Instruction has parameters.
	 */
	virtual bool isParameterized() {
		return true;
	}
	/**
	 * Returns true if this Instruction is composite,
	 * ie, contains other Instructions.
	 *
	 * @return isComposite True if this is a composite Instruction
	 */
	virtual bool isComposite() {
		return false;
	}

	/**
	 * Returns true if this Instruction is enabled
	 *
	 * @return enabled True if this Instruction is enabled.
	 */
	virtual bool isEnabled() {
		return true;
	}

	/**
	 * Disable this Instruction
	 */

	virtual void disable() {
	}

	/**
	 * Enable this Instruction.
	 */
	virtual void enable() {
	}

	/**
	 * Overloaded operator to check equality between this SpinInstruction
	 * and CompositeSpinInstructions
	 * @param b the composite spin instruction
	 * @return equal
	 */
	bool operator ==(CompositeSpinInstruction &b) const {
		if (b.nInstructions() > 1) {
			return false;
		} else {
			auto casted = std::dynamic_pointer_cast<SpinInstruction>(b.getInstruction(0));
			return operator==(*casted.get());
		}
	}

	/**
	 * Overloaded operator to check this SpinInstruction is not
	 * equal to the given CompositeSpinInstruction
	 *
	 * @param b the composite spin instruction
	 * @return notEqual
	 */
	bool operator !=(CompositeSpinInstruction &b) const {
		return !operator==(b);
	}

	const std::string id() const {
		if (isIdentity()) {
			return "I";
		} else {
			std::stringstream s;
			for (auto& t : terms) {
				if (t.second != "I")
					s << t.second << t.first;
			}
			return s.str();
		}
	}
	/**
	 * Return true if the given SpinInstruction is equal to this one
	 *
	 * @param b Other SpinInstruction
	 * @return equal
	 */
	bool operator ==(const SpinInstruction &b) const {
		return variable == b.variable && id() == b.id();
	}

	/**
	 * Return true if this SpinInstruction is not equal to the provided one
	 * @param b The other SpinInstruction
	 * @return equal
	 */
	bool operator !=(const SpinInstruction &b) const {
		return !operator==(b);
	}

	/**
	 * Multiply this SpinInstruction by the given one, and
	 * return a new SpinInstruction instance.
	 *
	 * @param b The other SpinInstruciton
	 * @return multSpinInstruction
	 */
	SpinInstruction operator*(const SpinInstruction &b) const {

		auto newCoeff = coefficient * b.coefficient;

		std::stringstream ss;
		if (!variable.empty()) {
			if (!b.variable.empty()) {
				ss << variable << " " << b.variable;
			} else {
				ss << variable;
			}
		} else {
			if(!b.variable.empty()) {
				ss << b.variable;
			}
		}

		auto newVar = ss.str();

		CommonPauliProducts products;
		std::map<int, std::string> newTerms = terms;
		for (auto& kv : b.terms) {
			if (newTerms.count(kv.first)) {
				// This means, we have a Pi in both
				// so we need to check its product
				auto gate_coeff = products[{newTerms[kv.first], kv.second}];
				if (gate_coeff.second != "I") {
					newTerms[kv.first] = gate_coeff.second;
				} else {
					newTerms.erase(kv.first);
				}
				newCoeff *= gate_coeff.first;
			} else if (kv.second != "I"){
				newTerms[kv.first] = kv.second;
			}
		}

		return SpinInstruction(newTerms, newCoeff, newVar);
	}

	/**
	 * Multiply this SpinInstruction by the given scalar complex value
	 * and return the result as a new SpinInstruction
	 *
	 * @param b the value
	 * @return multipliedInst
	 */
	SpinInstruction operator*(const std::complex<double> &b) const {
		SpinInstruction ret(*this);
		ret.coefficient = coefficient * b;
		return ret;
	}

	/**
	 * Multiply this SpinInstruction by the given scalar double value
	 * and return the result as a new SpinInstruction
	 *
	 * @param b the value
	 * @return multipliedInst
	 */

	SpinInstruction operator*(const double &b) const {
		return operator*(std::complex<double>(b, 0.0));
	}

	/**
	 * Multiply this SpinInstruction by the given scalar double value
	 * and return a reference to this SpinInstruction
	 *
	 * @param b the value
	 * @return multipliedInst
	 */
	SpinInstruction& operator*=(const double &d) {
		return operator*=(std::complex<double>(d, 0.0));
	}

	/**
	 * Multiply this SpinInstruction by the given scalar complex value
	 * and return a reference to this SpinInstruction
	 *
	 * @param b the value
	 * @return multipliedInst
	 */
	SpinInstruction& operator*=(const std::complex<double> &d) {
		coefficient *= d;
		return *this;
	}

	/**
	 * Multiply this SpinInstruction by the given CompositeSpinInstruction
	 * and return a new CompositeSpinInstruction.
	 *
	 * @param b the CompositeSpinInstruction
	 * @return multipliedInst
	 */
	CompositeSpinInstruction operator*(CompositeSpinInstruction &b) const {

		// Multiply a single term by ( sum of terms... )
		CompositeSpinInstruction ret;

		for (auto i : b.getInstructions()) {
			auto casted = std::dynamic_pointer_cast<SpinInstruction>(i);
			auto newspinInst = operator*(*casted.get());
			ret.addInstruction(std::make_shared<SpinInstruction>(newspinInst));
		}

		ret.simplify();

		return ret;
	}

	/**
	 * Add the given SpinInstruction to this SpinInstruction,
	 * creating a new CompositeSpinInstruction.
	 *
	 * @param b The instruction to add to this one
	 * @return addCompInst The sum result.
	 */
	CompositeSpinInstruction operator+(const SpinInstruction &b) const {
		CompositeSpinInstruction ret;
		ret.addInstruction(std::make_shared<SpinInstruction>(*this));
		ret.addInstruction(std::make_shared<SpinInstruction>(b));
		ret.simplify();
		return ret;
	}

	std::map<int, std::string> getTerms() {
		return terms;
	}

	/**
	 * We don't want this Instruction to be visitable
	 * @param EMPTY_DEFINE_VISITABLE()
	 */
	EMPTY_DEFINE_VISITABLE()

};
}
}

/**
 * Operator to enable lhs multiplication by scalar
 * @param scalar
 * @param rhs
 * @return
 */
xacc::vqe::SpinInstruction operator*(double const& scalar,
		xacc::vqe::SpinInstruction rhs);

xacc::vqe::SpinInstruction operator*(std::complex<double> const& scalar,
		xacc::vqe::SpinInstruction rhs);

#endif