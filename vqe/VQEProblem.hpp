#ifndef VQE_VQEPROBLEM_HPP_
#define VQE_VQEPROBLEM_HPP_

#include "problem.h"
#include "XACC.hpp"

#include "VQEGateFunction.hpp"

namespace xacc {

namespace vqe {

template<typename T>
class VQEProblem : public cppoptlib::Problem<T> {

protected:

	std::shared_ptr<AcceleratorBuffer> buffer;

	std::vector<Kernel<>> kernels;

	int nParameters;

	std::ofstream file;

	bool valueExecutedOnce = false;
public:

	using typename cppoptlib::Problem<T>::TVector;

	~VQEProblem() {file.close();}

	VQEProblem(std::istream& moleculeKernel) :nParameters(0), file("H2_Qasm_R_39.qasm") {

		auto serviceRegistry = xacc::ServiceRegistry::instance();
		auto runtimeOptions = RuntimeOptions::instance();
		(*runtimeOptions)["compiler"] = "fermion";
		runtimeOptions->insert(std::make_pair("state-preparation", "uccsd"));
		runtimeOptions->insert(std::make_pair("n-electrons", "2"));

		// Create the Accelerator
		auto qpu = xacc::getAccelerator("tnqvm");

		// Create the Program
		xacc::Program program(qpu, moleculeKernel);

		// Start compilation
		program.build();

		// Create a buffer of qubits
		std::string nQbtsStr = (*runtimeOptions)["n-qubits"];
		buffer = qpu->createBuffer("qreg", std::stoi(nQbtsStr));

		// Get the Kernels that were created
		kernels = program.getRuntimeKernels();

		// Set the number of VQE parameters
		nParameters = kernels[0].getNumberOfKernelParameters();
	}

	typename cppoptlib::Problem<T>::TVector initializeParameters() {
		Eigen::VectorXd x(nParameters);
		x << .45, -.04;
		return x;
	}


	T value(const TVector& x) {

		std::vector<InstructionParameter> parameters;
		for (int i = 0; i < x.rows(); i++) {
			InstructionParameter p(x(i));
			parameters.push_back(p);
		}

		double sum = 0.0;
		for (int i = 0; i < kernels.size(); i++) {
			double expectationValue = 0.0;
			auto kernel = kernels[i];
			auto vqeFunction = std::dynamic_pointer_cast<VQEGateFunction>(
							kernel.getIRFunction());

			if (!valueExecutedOnce) {
				auto str = vqeFunction->toString("qreg");
				file << "\n\nKernel " << std::to_string(i) << " Qasm:\n" << str;
			}

			if (vqeFunction->nInstructions() > 0) {
				kernel(buffer, parameters);
				// Get Expectation value
				expectationValue = buffer->getExpectationValueZ();
			} else {
				expectationValue = 1.0;
			}

			sum += vqeFunction->coefficient * expectationValue;

			buffer->resetBuffer();
		}

		valueExecutedOnce = true;

		XACCInfo("Computed VQE Energy = " + std::to_string(sum));
		return sum;
	}
};
}

}

#endif


//auto vis = std::make_shared<xacc::vqe::PrintScaffoldVisitor>("qreg");
//
//		auto f = kernels[10].getIRFunction();
//		f->evaluateVariableParameters(parameters);
//
//		InstructionIterator it(f);
//		while (it.hasNext()) {
//			// Get the next node in the tree
//			auto nextInst = it.next();
//			if (nextInst->isEnabled())
//				nextInst->accept(vis);
//		}
//
//		std::cout << "SCAFFOLD SRC:\n" << vis->getScaffoldString() << "\n";
