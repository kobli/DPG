#!/usr/bin/python
primInLeafCount = 10000
executable = "Release/FrustumCulling.exe"
statsFilePath = "../stats/"

viewNames = ["A10.view", "city_B.view", "conference_B.view", "teapots_B.view", "asianDragon_B.view", "fforest_B.view", "city2_B.view"]
noOptimFlags = ["", "-no-cotant-test", "-no-plane-test", "-no-plane-masking", "-no-cotant-test -no-plane-test -no-plane-masking"]
noOptimFileSuffixes = ["", "o", "l", "m", "olm"]

for viewFileName in viewNames:
    with open("data/scenes/"+viewFileName, 'r') as file:
        viewFlags  = file.read().replace('\n', '')
    for noOptimFlag, optDis in zip(noOptimFlags, noOptimFileSuffixes):
        viewName = viewFileName[:-5]
        statsFile = statsFilePath+"%s_%s.stats"%(viewName, optDis)
        print("%s %s -q -c %s -m %s %s"%(executable, viewFlags, primInLeafCount, statsFile, noOptimFlag))

