########### install files ###############

install(
  FILES
    AbstractLocation.h
    binary_analysis.h
    BinaryAnalysisUtils.h
    BinaryCallingConvention.h
    BinaryControlFlow.h
    BinaryDataFlow.h
    BinaryDominance.h
    BinaryFunctionCall.h
    BinaryPointerDetection.h
    BinaryTaintedFlow.h
    dataflowanalyses/InterruptAnalysis.h
    dataflowanalyses/RoseBin_DataFlowAbstract.h
    dataflowanalyses/RoseBin_DefUseAnalysis.h
    dataflowanalyses/RoseBin_Emulate.h
    dataflowanalyses/RoseBin_VariableAnalysis.h
    DwarfLineMapper.h
    ether.h
    GraphAlgorithms.h
    instructionSemantics/BaseSemantics2.h
    instructionSemantics/BaseSemantics.h
    instructionSemantics/DataFlowSemantics2.h
    instructionSemantics/DispatcherM68k.h
    instructionSemantics/DispatcherPowerpc.h
    instructionSemantics/DispatcherX86.h
    instructionSemantics/findConstants.h
    instructionSemantics/FindRegisterDefs.h
    instructionSemantics/flowEquations.h
    instructionSemantics/InsnSemanticsExpr.h
    instructionSemantics/IntervalSemantics2.h
    instructionSemantics/IntervalSemantics.h
    instructionSemantics/MultiSemantics2.h
    instructionSemantics/MultiSemantics.h
    instructionSemantics/NullSemantics2.h
    instructionSemantics/NullSemantics.h
    instructionSemantics/PartialSymbolicSemantics2.h
    instructionSemantics/PartialSymbolicSemantics.h
    instructionSemantics/ReadWriteRegisterFragment.h
    instructionSemantics/SMTSolver.h
    instructionSemantics/SymbolicSemantics2.h
    instructionSemantics/SymbolicSemantics.h
    instructionSemantics/TestSemantics2.h
    instructionSemantics/TraceSemantics2.h
    instructionSemantics/x86InstructionSemantics.h
    instructionSemantics/YicesSolver.h
    libraryIdentification/functionIdentification.h
    libraryIdentification/libraryIdentification.h
    RoseBin_CallGraphAnalysis.h
    RoseBin_CompareAnalysis.h
    RoseBin_ControlFlowAnalysis.h
    RoseBin_DataFlowAnalysis.h
    RoseBin_FlowAnalysis.h
    RoseBin_printInstructions.h
  DESTINATION include)
