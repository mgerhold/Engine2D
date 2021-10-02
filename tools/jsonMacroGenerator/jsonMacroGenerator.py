maxNumMembers = 64

for i in range(1, maxNumMembers + 1):
    parameterList = ", ".join([f"m{x}" for x in range(1, i + 1)])
    print(f"#define C2K_JSON_IMPLEMENTATION_TO_JSON_MEMBERS{i}({parameterList})\t\\")
    print("\tC2K_JSON_IMPLEMENTATION_TO_JSON_MEMBER(m1)", end='')
    for j in range(2, i + 1):
        print(f",\t\\\n\tC2K_JSON_IMPLEMENTATION_TO_JSON_MEMBER(m{j})", end='')
    print()
print()

for i in range(1, maxNumMembers + 1):
    parameterList = "TYPE, " + ", ".join([f"m{x}" for x in range(1, i + 1)])
    print(f"#define C2K_JSON_IMPLEMENTATION_FROM_JSON_MEMBERS{i}({parameterList})\t\\")
    print("\tC2K_JSON_IMPLEMENTATION_FROM_JSON_MEMBER(TYPE, m1)", end='')
    for j in range(2, i + 1):
        print(f"\t\\\n\tC2K_JSON_IMPLEMENTATION_FROM_JSON_MEMBER(TYPE, m{j})", end='')
    print()
print()

parameterList = ", ".join([f"_{x}" for x in range(1, maxNumMembers + 1)]) + ", NAME, ..."
print(f"#define C2K_JSON_IMPLEMENTATION_GET_MACRO({parameterList}) NAME")
print()

print("#define C2K_JSON_IMPLEMENTATION_TO_JSON_MEMBERS(...)\t\\")
print(
    "\tC2K_JSON_IMPLEMENTATION_PASS_ON(C2K_JSON_IMPLEMENTATION_PASS_ON(C2K_JSON_IMPLEMENTATION_PASS_ON(C2K_JSON_IMPLEMENTATION_PASS_ON(C2K_JSON_IMPLEMENTATION_GET_MACRO)(__VA_ARGS__,\t\\")
for i in reversed(range(1, maxNumMembers + 1)):
    print(f"\t\tC2K_JSON_IMPLEMENTATION_TO_JSON_MEMBERS{i},\t\\")
print("\t)))(__VA_ARGS__))")
print()

print("#define C2K_JSON_IMPLEMENTATION_FROM_JSON_MEMBERS(TYPE, ...)\t\\")
print(
    "\tC2K_JSON_IMPLEMENTATION_PASS_ON(C2K_JSON_IMPLEMENTATION_PASS_ON(C2K_JSON_IMPLEMENTATION_PASS_ON(C2K_JSON_IMPLEMENTATION_PASS_ON(C2K_JSON_IMPLEMENTATION_GET_MACRO)(__VA_ARGS__,\t\\")
for i in reversed(range(1, maxNumMembers + 1)):
    print(f"\t\tC2K_JSON_IMPLEMENTATION_FROM_JSON_MEMBERS{i},\t\\")
print("\t)))(TYPE, __VA_ARGS__))")
print()
