# Cyber-Spider

Cyber Spider program crawls through a list of known interactions between different websites or files (called logs) to determine which are malicious and caused cyberattacks and which are harmless.

To use this function, give the program a text file of known malicious files and websites and a text file of any logs. The latter must be in the form of: "<entity1>, <entity2>", and each on separate lines. The program will then create a text file containing the known malicious sites and files and all potentially malicious entities that the known ones interacted with, while disregarding the ones that are frequently used, such as google.com. 
