# Parser Completion Issue List

Goal: make `build/svt test/all.sv` produce a fully modeled AST with no
`<unsupported>` entries.

This file tracks parser work in small increments. Each issue should preserve
the current recovery behavior: if a construct is only partly implemented, the
parser must still resynchronize and continue parsing later supported syntax.

## Validation

Use this command as the main progress check:

```shell
build/svt test/all.sv
```

An issue is complete when the targeted syntax no longer prints as
`<unsupported>` in `test/all.sv`, focused parser tests cover the new AST shape,
and existing parser tests still pass.

## Issues

### 1. Add top-level time declarations (done)

- Parse timeunit and timeprecision as first-class compilation-unit items.
- Model optional precision syntax such as 1ns / 1ps.
- Update the AST printer so these no longer appear as unsupported top-level
elements.
- Add focused unit tests for timeunit, timeprecision, and continued top-level
parsing after time declarations.

### 2. Add package declarations (done)

- Parse `package ... endpackage`, including optional attributes and lifetime.
- Model package items needed by `test/all.sv`: `timeunit`, parameters,
  nested unsupported recovery items, imports, exports, and package-scope
  declarations.
- Keep unsupported package items scoped inside the package instead of surfacing
  as separate top-level unsupported elements.

### 3. Add import and export declarations (done)

- Parse top-level, package-level, and module-level `import` declarations.
- Parse package exports, including explicit symbols and wildcard exports.
- Reuse a common package-scope name representation for paths like `p::x` and
  `*::*`.

### 4. Expand module header support (done)

- Parse module lifetime (`automatic` / `static`) and module-header imports.
- Parse non-ANSI and mixed port lists without turning later `input`/`output`
  declarations into unsupported module items.
- Model empty positional ports and explicit named ports such as `.e()`.

### 5. Expand port declarations (done)

- Add directions beyond `input` and `output`: `inout` and `ref`.
- Parse port data types, packed dimensions, unpacked dimensions, default
  values, attributes, and interface/modport ports.
- Preserve port declaration details in the AST instead of only name and
  direction.

### 6. Add data declarations beyond wire and logic (done)

- Parse common variable declarations: `reg`, `int`, `integer`, `shortint`,
  `longint`, `byte`, `bit`, `real`, `time`, `shortreal`, `chandle`,
  `realtime`, `event`, and `string`.
- Support declaration assignments, multiple declarators, packed dimensions,
  unpacked dimensions, queues, dynamic arrays, associative arrays, and
  attributes.
- Replace the current keyword-based `UnsupportedModuleItem` fallback for these
  declaration forms.

### 7. Add parameter and localparam declarations as module items (done)

- Parse `parameter` and `localparam` declarations outside parameter lists.
- Support value parameters, type parameters, typed parameters, and multiple
  declarators.
- Share as much AST shape as possible with existing parameter-list parsing.

### 8. Add type declarations

- Parse `typedef`, `enum`, `struct`, `union`, tagged unions, forward class
  typedefs, and scoped type references.
- Parse `nettype` declarations, including optional resolution functions.
- Allow user-defined net types in declarations and module items.

### 9. Add interface declarations

- Parse `interface ... endinterface` as a top-level design element.
- Model interface ports, interface items, modports, extern tasks/functions, and
  default clocking declarations, and time declarations.
- Support interface instances and modport references used as module ports.

### 10. Add program, primitive, and macromodule declarations

- Parse `program ... endprogram`.
- Model program time declarations and program items.
- Parse UDP `primitive ... endprimitive`, including ports, `table`, and
  optional initial statements.
- Treat `macromodule` as a module-like declaration with its own source kind.

### 11. Add class declarations

- Parse top-level, module-level, and nested class declarations.
- Model inheritance, parameterized classes, fields, methods, extern methods,
  constraints, random qualifiers, constructors, and scoped method definitions.
- Support class handles and `new` expressions enough for `test/all.sv`.

### 12. Add subroutine declarations

- Parse functions and tasks in modules, interfaces, classes, and compilation
  units.
- Support extern declarations, scoped names, lifetimes, return types, ports,
  default arguments, and empty bodies.
- Model DPI/import-export forms separately from normal subroutines.

### 13. Add specify blocks

- Parse `specify ... endspecify`.
- Model `specparam`, path declarations, edge-sensitive paths, conditional
  paths, polarity operators, and timing value lists.
- Keep specify items isolated so unsupported specify syntax does not consume
  following module items.

### 14. Add assertions, properties, and sequences

- Parse `property`, `sequence`, `assert`, `assume`, `cover`, `restrict`, and
  labels on concurrent assertions.
- Support disable conditions, property/sequence ports, implication operators,
  repetition operators, `matches`, `throughout`, `within`, and action blocks
  as they appear in `test/all.sv`.
- Start with token-preserving AST nodes where full semantic expression modeling
  would be too large.

### 15. Add clocking declarations and default directives

- Parse `clocking`, `global clocking`, `default clocking`, and
  `default disable iff`.
- Model clocking events, default skews, input/output clockvars, and labels.
- Ensure these parse both in modules and checkers.

### 16. Add checker declarations

- Parse `checker ... endchecker` as a top-level design element.
- Model checker ports, time declarations, default clocking/disable
  declarations, local declarations, procedural blocks, assertions, and generate
  blocks.
- Support checker instantiations in module and procedural contexts.

### 17. Add generate item coverage

- Complete generate `for`, `if`, and `case` support for the forms in
  `test/all.sv`.
- Support named generate blocks, generate regions, nested declarations,
  assertions, checker instances, and null generate items.
- Remove remaining `UnsupportedGenerateItem` output.

### 18. Add module instantiation coverage

- Parse instance arrays and primitive gate/switch instances.
- Parse drive strengths, delays, empty ports, wildcard ports, named ports, and
  parameter overrides.
- Distinguish module instances, interface instances, checker instances, and
  primitive/gate instances in the AST.

### 19. Add bind, alias, defparam, and let declarations

- Parse `bind` directives at compilation-unit and module-item scope.
- Parse `alias`, `defparam`, and `let` declarations.
- Model hierarchical names, parameter assignments, and let ports/bodies.

### 20. Expand statement parsing

- Add procedural declarations inside blocks.
- Parse event triggers, nonblocking event triggers, procedural timing controls,
  `expect`, `randsequence`, `return`, `break`, `continue`, `disable`,
  procedural `assign/deassign/force/release`, and block labels.
- Preserve existing structured statements while replacing unsupported statement
  token spans incrementally.

### 21. Expand expression parsing

- Add casts, scoped names, hierarchical names, member access, method calls,
  streaming concatenations, assignment patterns with keys, empty
  concatenations, inside/dist expressions, matches expressions, type
  expressions, min/typ/max expressions, and event expressions.
- Support all operators exercised by `test/all.sv` with correct precedence.
- Keep token spans on expression nodes for printer/debug parity.

### 22. Add covergroups

- Parse `covergroup ... endgroup` in class scope.
- Model coverpoints, crosses, bins, ignore bins, wildcard bins, transition
  bins, options, `iff`, `with`, and sample functions.
- Support event controls and unusual event expressions used by the fixture.

### 23. Add config declarations

- Parse `config ... endconfig`.
- Model `design`, `default liblist`, `cell use`, and `instance liblist/use`
  clauses.
- Keep config parsing at compilation-unit scope.

### 24. Improve AST printer coverage

- For each new AST node, add printer output that is stable enough for manual
  inspection.
- Avoid printing `<unsupported>` for nodes that are structurally represented,
  even if subexpressions are still token-preserving.
- Consider adding a summary mode that counts remaining unsupported nodes.

### 25. Add all.sv regression guard

- Add a test that parses `test/all.sv` and asserts there are no unsupported
  AST nodes once the roadmap is complete.
- Until then, add a non-failing diagnostic or expected-count test that makes
  unsupported count changes visible.
- Keep smaller focused tests for each issue so failures identify the grammar
  area that regressed.

### 26. Add semantic time literal AST nodes

- Replace `TimeDeclaration` token spans for `time_value` and `precision_value`
  with a typed time-literal representation.
- Split each time literal into magnitude and unit fields, preserving the
  original token span for diagnostics and printer/debug output.
- Validate legal time units and the allowed `timeunit` / `timeprecision`
  declaration forms.
- Reuse the same representation for later time-valued syntax such as delays
  and specify timing values where appropriate.
