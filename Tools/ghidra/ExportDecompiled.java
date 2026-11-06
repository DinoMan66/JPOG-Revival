// Ghidra headless post-script: writes the decompiled C of EVERY function in the program to
//   <output folder>\<program name>.c
// Usage (via tools\export_decompiled.bat):   -postScript ExportDecompiled.java <output folder>
//
// @category JPOG
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import java.io.File;
import java.io.PrintWriter;

public class ExportDecompiled extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        File dir = new File(args.length > 0 ? args[0] : "decompiled");
        dir.mkdirs();
        File out = new File(dir, currentProgram.getName() + ".c");

        DecompInterface ifc = new DecompInterface();
        ifc.openProgram(currentProgram);

        int total = 0, failed = 0;
        try (PrintWriter w = new PrintWriter(out, "UTF-8")) {
            w.println("// Decompiled by Ghidra from " + currentProgram.getName()
                    + " (image base " + currentProgram.getImageBase() + ")");
            FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
            while (it.hasNext() && !monitor.isCancelled()) {
                Function fn = it.next();
                total++;
                w.println();
                w.println("// ==== " + fn.getEntryPoint() + "  " + fn.getName(true) + "  (size "
                        + fn.getBody().getNumAddresses() + " bytes)");
                DecompileResults res = ifc.decompileFunction(fn, 60, monitor);
                if (res.decompileCompleted() && res.getDecompiledFunction() != null) {
                    w.println(res.getDecompiledFunction().getC());
                } else {
                    failed++;
                    w.println("// (decompilation failed: " + res.getErrorMessage() + ")");
                }
            }
        }
        println("ExportDecompiled: wrote " + total + " functions (" + failed + " failed) to " + out);
    }
}
