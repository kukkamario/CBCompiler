#include "llc.h"
#include "llvm/ADT/Triple.h"
#include "llvm/CodeGen/LinkAllAsmWriterComponents.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PluginLoader.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Target/TargetMachine.h"
#include <memory>

#include "settings.h"

#include "llvm/MC/MCTargetOptionsCommandFlags.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <string>

#include <QDir>
using namespace llvm;

cl::opt<std::string>
MArch("march", cl::desc("Architecture to generate code for (see --version)"));

cl::opt<std::string>
MCPU("mcpu",
	 cl::desc("Target a specific cpu type (-mcpu=help for details)"),
	 cl::value_desc("cpu-name"),
	 cl::init(""));

cl::list<std::string>
MAttrs("mattr",
	   cl::CommaSeparated,
	   cl::desc("Target specific attributes (-mattr=help for details)"),
	   cl::value_desc("a1,+a2,-a3,..."));

cl::opt<Reloc::Model>
RelocModel("relocation-model",
		   cl::desc("Choose relocation model"),
		   cl::init(Reloc::Default),
		   cl::values(
			  clEnumValN(Reloc::Default, "default",
					  "Target default relocation model"),
			  clEnumValN(Reloc::Static, "static",
					  "Non-relocatable code"),
			  clEnumValN(Reloc::PIC_, "pic",
					  "Fully relocatable, position independent code"),
			  clEnumValN(Reloc::DynamicNoPIC, "dynamic-no-pic",
					  "Relocatable external references, non-relocatable code"),
			  clEnumValEnd));

cl::opt<llvm::CodeModel::Model>
CMModel("code-model",
		cl::desc("Choose code model"),
		cl::init(CodeModel::Default),
		cl::values(clEnumValN(CodeModel::Default, "default",
							  "Target default code model"),
				   clEnumValN(CodeModel::Small, "small",
							  "Small code model"),
				   clEnumValN(CodeModel::Kernel, "kernel",
							  "Kernel code model"),
				   clEnumValN(CodeModel::Medium, "medium",
							  "Medium code model"),
				   clEnumValN(CodeModel::Large, "large",
							  "Large code model"),
				   clEnumValEnd));

cl::opt<bool>
DisableRedZone("disable-red-zone",
			   cl::desc("Do not emit code that uses the red zone."),
			   cl::init(false));

cl::opt<bool>
EnableFPMAD("enable-fp-mad",
			cl::desc("Enable less precise MAD instructions to be generated"),
			cl::init(false));

cl::opt<bool>
DisableFPElim("disable-fp-elim",
			  cl::desc("Disable frame pointer elimination optimization"),
			  cl::init(false));

cl::opt<bool>
EnableUnsafeFPMath("enable-unsafe-fp-math",
				cl::desc("Enable optimizations that may decrease FP precision"),
				cl::init(false));

cl::opt<bool>
EnableNoInfsFPMath("enable-no-infs-fp-math",
				cl::desc("Enable FP math optimizations that assume no +-Infs"),
				cl::init(false));

cl::opt<bool>
EnableNoNaNsFPMath("enable-no-nans-fp-math",
				   cl::desc("Enable FP math optimizations that assume no NaNs"),
				   cl::init(false));

cl::opt<bool>
EnableHonorSignDependentRoundingFPMath("enable-sign-dependent-rounding-fp-math",
	  cl::Hidden,
	  cl::desc("Force codegen to assume rounding mode can change dynamically"),
	  cl::init(false));

cl::opt<bool>
GenerateSoftFloatCalls("soft-float",
					cl::desc("Generate software floating point library calls"),
					cl::init(false));

cl::opt<llvm::FloatABI::ABIType>
FloatABIForCalls("float-abi",
				 cl::desc("Choose float ABI type"),
				 cl::init(FloatABI::Default),
				 cl::values(
					 clEnumValN(FloatABI::Default, "default",
								"Target default float ABI type"),
					 clEnumValN(FloatABI::Soft, "soft",
								"Soft float ABI (implied by -soft-float)"),
					 clEnumValN(FloatABI::Hard, "hard",
								"Hard float ABI (uses FP registers)"),
					 clEnumValEnd));

cl::opt<llvm::FPOpFusion::FPOpFusionMode>
FuseFPOps("fp-contract",
		  cl::desc("Enable aggressive formation of fused FP ops"),
		  cl::init(FPOpFusion::Standard),
		  cl::values(
			  clEnumValN(FPOpFusion::Fast, "fast",
						 "Fuse FP ops whenever profitable"),
			  clEnumValN(FPOpFusion::Standard, "on",
						 "Only fuse 'blessed' FP ops."),
			  clEnumValN(FPOpFusion::Strict, "off",
						 "Only fuse FP ops when the result won't be effected."),
			  clEnumValEnd));

cl::opt<bool>
DontPlaceZerosInBSS("nozero-initialized-in-bss",
			  cl::desc("Don't place zero-initialized symbols into bss section"),
			  cl::init(false));

cl::opt<bool>
EnableGuaranteedTailCallOpt("tailcallopt",
  cl::desc("Turn fastcc calls into tail calls by (potentially) changing ABI."),
  cl::init(false));

cl::opt<bool>
DisableTailCalls("disable-tail-calls",
				 cl::desc("Never emit tail calls"),
				 cl::init(false));

cl::opt<unsigned>
OverrideStackAlignment("stack-alignment",
					   cl::desc("Override default stack alignment"),
					   cl::init(0));

cl::opt<std::string>
TrapFuncName("trap-func", cl::Hidden,
		cl::desc("Emit a call to trap function rather than a trap instruction"),
		cl::init(""));

cl::opt<bool>
EnablePIE("enable-pie",
		  cl::desc("Assume the creation of a position independent executable."),
		  cl::init(false));

cl::opt<bool>
UseInitArray("use-init-array",
			 cl::desc("Use .init_array instead of .ctors."),
			 cl::init(false));

cl::opt<std::string> StopAfter("stop-after",
							cl::desc("Stop compilation after a specific pass"),
							cl::value_desc("pass-name"),
									  cl::init(""));
cl::opt<std::string> StartAfter("start-after",
						  cl::desc("Resume compilation after a specific pass"),
						  cl::value_desc("pass-name"),
						  cl::init(""));

cl::opt<bool> DataSections("data-sections",
						   cl::desc("Emit data into separate sections"),
						   cl::init(false));

cl::opt<bool>
FunctionSections("function-sections",
				 cl::desc("Emit functions into separate sections"),
				 cl::init(false));

cl::opt<llvm::JumpTable::JumpTableType>
JTableType("jump-table-type",
		  cl::desc("Choose the type of Jump-Instruction Table for jumptable."),
		  cl::init(JumpTable::Single),
		  cl::values(
			  clEnumValN(JumpTable::Single, "single",
						 "Create a single table for all jumptable functions"),
			  clEnumValN(JumpTable::Arity, "arity",
						 "Create one table per number of parameters."),
			  clEnumValN(JumpTable::Simplified, "simplified",
						 "Create one table per simplified function type."),
			  clEnumValN(JumpTable::Full, "full",
						 "Create one table per unique function type."),
			  clEnumValEnd));

// Common utility function tightly tied to the options listed here. Initializes
// a TargetOptions object with CodeGen flags and returns it.
static inline TargetOptions InitTargetOptionsFromCodeGenFlags() {
	TargetOptions Options;
	Options.LessPreciseFPMADOption = EnableFPMAD;
	Options.NoFramePointerElim = DisableFPElim;
	Options.AllowFPOpFusion = FuseFPOps;
	Options.UnsafeFPMath = EnableUnsafeFPMath;
	Options.NoInfsFPMath = EnableNoInfsFPMath;
	Options.NoNaNsFPMath = EnableNoNaNsFPMath;
	Options.HonorSignDependentRoundingFPMathOption =
			EnableHonorSignDependentRoundingFPMath;
	Options.UseSoftFloat = GenerateSoftFloatCalls;
	if (FloatABIForCalls != FloatABI::Default)
		Options.FloatABIType = FloatABIForCalls;
	Options.NoZerosInBSS = DontPlaceZerosInBSS;
	Options.GuaranteedTailCallOpt = EnableGuaranteedTailCallOpt;
	Options.DisableTailCalls = DisableTailCalls;
	Options.StackAlignmentOverride = OverrideStackAlignment;
	Options.TrapFuncName = TrapFuncName;
	Options.PositionIndependentExecutable = EnablePIE;
	Options.UseInitArray = UseInitArray;
	Options.DataSections = DataSections;
	Options.FunctionSections = FunctionSections;

	Options.MCOptions = InitMCTargetOptionsFromFlags();
	Options.JTType = JTableType;

	return Options;
}

static cl::opt<bool>
NoIntegratedAssembler("no-integrated-as", cl::Hidden,
					  cl::desc("Disable integrated assembler"));

// Determine optimization level.
cl::opt<char>
OptLevel("O",
		 cl::desc("Optimization level. [-O0, -O1, -O2, or -O3] "
				  "(default = '-O2')"),
		 cl::Prefix,
		 cl::ZeroOrMore,
		 cl::init(' '));

static cl::opt<bool> NoVerify("disable-verify", cl::Hidden,
							  cl::desc("Do not verify input module"));

static cl::opt<bool> DisableSimplifyLibCalls("disable-simplify-libcalls",
											 cl::desc("Disable simplify-libcalls"));

static cl::opt<bool> ShowMCEncoding("show-mc-encoding", cl::Hidden,
									cl::desc("Show encoding in .s output"));

static cl::opt<bool> EnableDwarfDirectory(
		"enable-dwarf-directory", cl::Hidden,
		cl::desc("Use .file directives with an explicit directory."));

static cl::opt<bool> AsmVerbose("asm-verbose",
								cl::desc("Add comments to directives."),
								cl::init(true));


struct LLC::LLCImpl {
		LLCImpl(int argc, char *argv[]) :
			X(argc, argv),
			argv(argv),
			argc(argc){
		}
		llvm_shutdown_obj Y;
		PrettyStackTraceProgram X;
		char **argv;
		int argc;
};

LLC::LLC(int argc, char *argv[]) {
	sys::PrintStackTraceOnErrorSignal();
	impl = new LLC::LLCImpl(argc, argv);

	// Enable debug stream buffering.
	EnableDebugBuffering = true;

	LLVMContext &Context = getGlobalContext();

	// Initialize targets first, so that --version shows registered targets.
	InitializeAllTargets();
	InitializeAllTargetMCs();
	InitializeAllAsmPrinters();
	InitializeAllAsmParsers();

	// Initialize codegen and IR passes used by llc so that the -print-after,
	// -print-before, and -stop-after options work.
	PassRegistry *Registry = PassRegistry::getPassRegistry();
	initializeCore(*Registry);
	initializeCodeGen(*Registry);
	initializeLoopStrengthReducePass(*Registry);
	initializeLowerIntrinsicsPass(*Registry);
	initializeUnreachableBlockElimPass(*Registry);

	// Register the target printer for --version.
	cl::AddExtraVersionPrinter(TargetRegistry::printRegisteredTargetsForVersion);
}


// GetFileNameRoot - Helper function to get the basename of a filename.
static inline std::string GetFileNameRoot(const std::string &InputFilename) {
	std::string IFN = InputFilename;
	std::string outputFilename;
	int Len = IFN.length();
	if ((Len > 2) &&
			IFN[Len-3] == '.' &&
			((IFN[Len-2] == 'b' && IFN[Len-1] == 'c') ||
			 (IFN[Len-2] == 'l' && IFN[Len-1] == 'l'))) {
		outputFilename = std::string(IFN.begin(), IFN.end()-3); // s/.bc/.s/
	} else {
		outputFilename = IFN;
	}
	return outputFilename;
}

bool LLC::compileModule(Module *mod, std::string &outputFile) {
	char **argv = impl->argv;
	LLVMContext &Context = getGlobalContext();
	SMDiagnostic Err;
	Triple TheTriple;

	bool SkipModule = MCPU == "help" ||
			(!MAttrs.empty() && MAttrs.front() == "help");

	// If user asked for the 'native' CPU, autodetect here. If autodection fails,
	// this will set the CPU to an empty string which tells the target to
	// pick a basic default.
	if (MCPU == "native")
		MCPU = sys::getHostCPUName();

	TheTriple = Triple(mod->getTargetTriple());

	if (TheTriple.getTriple().empty())
		TheTriple.setTriple(sys::getDefaultTargetTriple());

	// Get the target specific parser.
	std::string Error;
	const Target *TheTarget = TargetRegistry::lookupTarget(MArch, TheTriple,
														   Error);
	if (!TheTarget) {
		errs() << argv[0] << ": " << Error;
		return false;
	}

	// Package up features to be passed to target/subtarget
	std::string FeaturesStr;
	if (MAttrs.size()) {
		SubtargetFeatures Features;
		for (unsigned i = 0; i != MAttrs.size(); ++i)
			Features.AddFeature(MAttrs[i]);
		FeaturesStr = Features.getString();
	}

	CodeGenOpt::Level OLvl = CodeGenOpt::Default;
	switch (OptLevel) {
		default:
			errs() << argv[0] << ": invalid optimization level.\n";
			return false;
		case ' ': break;
		case '0': OLvl = CodeGenOpt::None; break;
		case '1': OLvl = CodeGenOpt::Less; break;
		case '2': OLvl = CodeGenOpt::Default; break;
		case '3': OLvl = CodeGenOpt::Aggressive; break;
	}

	TargetOptions Options = InitTargetOptionsFromCodeGenFlags();
	Options.DisableIntegratedAS = NoIntegratedAssembler;
	Options.MCOptions.ShowMCEncoding = ShowMCEncoding;
	Options.MCOptions.MCUseDwarfDirectory = EnableDwarfDirectory;
	Options.MCOptions.AsmVerbose = AsmVerbose;

	std::unique_ptr<TargetMachine> target(
				TheTarget->createTargetMachine(TheTriple.getTriple(), MCPU, FeaturesStr,
											   Options, RelocModel, CMModel, OLvl));
	assert(target.get() && "Could not allocate target machine!");

	// If we don't have a module then just exit now. We do this down
	// here since the CPU/Feature help is underneath the target machine
	// creation.
	if (SkipModule)
		return true;

	assert(mod && "Should have exited if we didn't have a module!");
	TargetMachine &Target = *target.get();

	if (GenerateSoftFloatCalls)
		FloatABIForCalls = FloatABI::Soft;


	TargetMachine::CodeGenFileType FileType;
	switch (Settings::outputFileType()) {
		case Settings::Assembly:
			FileType = TargetMachine::CGFT_AssemblyFile;
			break;
		case Settings::Library:
		case Settings::Executable:
		case Settings::ObjectFile:
			FileType = TargetMachine::CGFT_ObjectFile;
			break;
		case Settings::Null:
			FileType = TargetMachine::CGFT_Null;
			break;
		default:
			assert(0 && "Invalid output filetype for llc");
	}

	outputFile = Settings::outputFile().toStdString();
	if (!Settings::compileOnly()) {
		outputFile = QDir::temp().filePath(
					QFileInfo(Settings::inputFile()).baseName() + QFileInfo(Settings::outputFile()).baseName())
				.toStdString();

	}

	bool Binary = false;
	switch (FileType) {
		case TargetMachine::CGFT_AssemblyFile:
			break;
		case TargetMachine::CGFT_ObjectFile:
		case TargetMachine::CGFT_Null:
			Binary = true;
			break;
	}

	// Open the file.
	std::string error;
	sys::fs::OpenFlags OpenFlags = sys::fs::F_None;
	if (!Binary)
		OpenFlags |= sys::fs::F_Text;
	std::unique_ptr<tool_output_file> Out(new tool_output_file(outputFile.c_str(), error,
												   OpenFlags));
	if (!error.empty()) {
		errs() << error << '\n';
		return false;
	}


	if (!Out) return false;

	// Build up all of the passes that we want to do to the module.
	PassManager PM;

	// Add an appropriate TargetLibraryInfo pass for the module's triple.
	TargetLibraryInfo *TLI = new TargetLibraryInfo(TheTriple);
	if (DisableSimplifyLibCalls)
		TLI->disableAllFunctions();
	PM.add(TLI);

	// Add the target data from the target machine, if it exists, or the module.
	if (const DataLayout *DL = Target.getDataLayout())
		mod->setDataLayout(DL);
	PM.add(new DataLayoutPass(mod));

	if (RelaxAll.getNumOccurrences() > 0 &&
			FileType != TargetMachine::CGFT_ObjectFile)
		errs() << argv[0]
				<< ": warning: ignoring -mc-relax-all because filetype != obj";

	{
		formatted_raw_ostream FOS(Out->os());

		AnalysisID StartAfterID = nullptr;
		AnalysisID StopAfterID = nullptr;
		const PassRegistry *PR = PassRegistry::getPassRegistry();
		if (!StartAfter.empty()) {
			const PassInfo *PI = PR->getPassInfo(StartAfter);
			if (!PI) {
				errs() << argv[0] << ": start-after pass is not registered.\n";
				return false;
			}
			StartAfterID = PI->getTypeInfo();
		}
		if (!StopAfter.empty()) {
			const PassInfo *PI = PR->getPassInfo(StopAfter);
			if (!PI) {
				errs() << argv[0] << ": stop-after pass is not registered.\n";
				return false;
			}
			StopAfterID = PI->getTypeInfo();
		}

		// Ask the target to add backend passes as necessary.
		if (Target.addPassesToEmitFile(PM, FOS, FileType, NoVerify,
									   StartAfterID, StopAfterID)) {
			errs() << argv[0] << ": target does not support generation of this"
							  << " file type!\n";
			return false;
		}

		// Before executing passes, print the final values of the LLVM options.
		cl::PrintOptionValues();

		PM.run(*mod);
	}

	// Declare success.
	Out->keep();

	return true;
}


