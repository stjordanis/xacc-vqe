from _pyxaccvqe import *
import xacc
import os, platform, sys, sysconfig
import argparse, inspect

def QubitOperator2XACC(qubit_op):
    """
    Map openfermion qubit operator to an XACC PauliOperator
    """
    xaccOp = PauliOperator()
    for k, v in qubit_op.terms.items():
        terms = dict(k)
        if terms:
            xaccOp += PauliOperator(terms, v)
        else:
            xaccOp += PauliOperator(v)
    return(xaccOp)


class qpu(xacc.qpu):
   class vqe(object):
       def __init__(self, *args, **kwargs):
           self.args = args
           self.kwargs = kwargs
           return
       def __call__(self, f):
          def wrapped_f(*args, **kwargs):
              src = '\n'.join(inspect.getsource(f).split('\n')[1:])
              compiler = xacc.getCompiler('xacc-py')
              if isinstance(self.kwargs['accelerator'], xacc.Accelerator):
                  qpu = self.kwargs['accelerator']
              else:
                  qpu = xacc.getAccelerator(self.kwargs['accelerator'])
              ir = compiler.compile(src, qpu)
              program = xacc.Program(qpu, ir)
              compiledKernel = program.getKernels()[0]
              getParams = lambda params: ','.join(map(str, params)) 

              execParams = {'accelerator':qpu.name(), 'ansatz':compiledKernel.getIRFunction(), 'task':'vqe'}
              obs = self.kwargs['observable']
              ars = list(args)

              if not isinstance(args[0], xacc.AcceleratorBuffer):
                  raise RuntimeError('First argument of an xacc kernel must be the Accelerator Buffer to operate on.')
             
              buffer = ars[0]
              ars = ars[1:]
              if len(ars) > 0:
                 arStr = getParams(ars)
                 execParams['vqe-params'] = arStr

              if 'optimizer' in self.kwargs:
                  optimizer = self.kwargs['optimizer']
                  if 'scipy-' in optimizer:
                      optimizer = optimizer.replace('scipy-','')
                      from scipy.optimize import minimize
                      execParams['task'] = 'compute-energy'
                      def energy(params):
                          pStr = getParams(params)
                          execParams['vqe-params'] = pStr
                          e = execute(obs, buffer, **execParams).energy
                          return e
                      if len(ars) == 0:
                          import random
                          pi = 3.141592653
                          ars = [random.uniform(-pi,pi) for _ in range(compiledKernel.getIRFunction().nParameters())]
                      optargs = {'method':optimizer, 'options':{'disp':True}}
                      if 'opt_params' in self.kwargs:
                          for k,v in self.kwargs['opt_params'].items():
                              optargs[k] = v
                      print(ars,compiledKernel.getIRFunction().nParameters() )
                      opt_result = minimize(energy, ars, **optargs)
                      return #VQETaskResult(opt_result.fun, opt_result.x)
                  else:
                      xacc.setOption('vqe-backend', optimizer)
                      if 'opt_params' in self.kwargs:
                          for k,v in self.kwargs['opt_params'].items():
                              xacc.setOption(k,str(v))
              execute(obs, buffer, **execParams)
              return
          return wrapped_f

   class energy(object):
       def __init__(self, *args, **kwargs):
           self.args = args
           self.kwargs = kwargs
           return
       def __call__(self, f):
          def wrapped_f(*args, **kwargs):
              src = '\n'.join(inspect.getsource(f).split('\n')[1:])
              compiler = xacc.getCompiler('xacc-py')
              if isinstance(self.kwargs['accelerator'], xacc.Accelerator):
                  qpu = self.kwargs['accelerator']
              else:
                  qpu = xacc.getAccelerator(self.kwargs['accelerator'])
              ir = compiler.compile(src, qpu)
              program = xacc.Program(qpu, ir)
              compiledKernel = program.getKernels()[0]
              getParams = lambda params: ','.join(map(str, params)) 

              execParams = {'accelerator':qpu.name(), 'ansatz':compiledKernel.getIRFunction(), 'task':'compute-energy'}
              obs = self.kwargs['observable']
              ars = list(args)

              if not isinstance(args[0], xacc.AcceleratorBuffer):
                  raise RuntimeError('First argument of an xacc kernel must be the Accelerator Buffer to operate on.')
             
              buffer = ars[0]
              ars = ars[1:]
              if len(ars) > 0:
                 arStr = getParams(ars)
                 execParams['vqe-params'] = arStr
              print(execParams)
              execute(obs, buffer, **execParams)
              return
          return wrapped_f

def main(argv=None):
   return
   
if __name__ == "__main__":
    sys.exit(main())
