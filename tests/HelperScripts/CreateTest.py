#!/usr/bin/python3

compTypes = 50
components = []

tagTypes = 14
tags = []

sigTypes = 128
signatures = []
signatureLists = []

for iii in range(compTypes) :
    name = "C" + str(iii)
    components.append(name)
    print("struct " + name + " {};")

print("using CompTypes = ecs::ComponentList<", end="")
for comp in components :
    print(comp + ", ", end="")
print(">;")

for iii in range(tagTypes) :
    name = "T" + str(iii)
    tags.append(name)
    print("struct " + name + " {};")

print("using TagTypes = ecs::TagList<", end="")
for tag in tags :
    print(tag + ", ", end="")
print(">;")

print("using S = ecs::Signature<", end="")
for comp in components :
    print(comp + ", ", end="")
for tag in tags :
    print(tag + ", ", end="")
print(">;")

print("using Signatures = ecs::SignatureList<S>;");

print("using CFG = ecs::Config<CompTypes, TagTypes, Signatures>;")

print("std::cout << \"Last Comp : \" << CFG::mask<" + components[-1] +">() << std::endl;")
print("std::cout << \"Last Tag : \" << CFG::mask<" + tags[-1] + ">() << std::endl;")
print("std::cout << \"Full Sig : \" << CFG::mask<S>() << std::endl;")

