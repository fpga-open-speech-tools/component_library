import os

additional_filesets = []

for filename in os.listdir('./'):
	if filename.endswith(".vhd") and '_avalon' not in filename and ('DSBF' in filename or "pkg" in filename):
	    additional_filesets.append(filename)
built_string = ""
for i in range(len(additional_filesets)):
        built_string += ("add_fileset_file " + additional_filesets[i] + " VHDL PATH " +
                         additional_filesets[i] + "\n")

print(built_string)