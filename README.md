# Real-Time Scout AI

My implementation of a scout AI to combat cheating AI in RTS games.

## Description

In most RTS games, there's a problem with cheating AI. The AI enemy usually knows the whole map from the start, making it unfair for newer players, or if you're playing on a random map, with no way to know or remember how the map looks beforehand. This gives the enemy a significant advantage.
To combat this, you can implement a scouting unit that maps the level for the AI, in the same way you would have to do it as the player. I've tried my hand at creating a simple implementation of such an AI scouting agent.

## Implementation

For the implementation, I will be using a framework provided to me by my teachers at the university of Howest, where I am studying. This framework provides me with a grid graph, and an agent that can move around by giving it a linear velocity. For the pathfinding, I will use an implementation of A* that I've made earlier.

![Research1](https://github.com/Louis-DV/Real-Time-Scout-AI/blob/master/Images/Research1.JPG "Research1")

Of course, the level needs different terrains in order for the scout to be able to actually do something worthwhile. I already had a few terrains in the framework, and I also made a few more myself. Now there's normal ground(grey), mud(brown), hills(light green), water(blue), and forests(dark green).
Normal ground doesn't do anything special, mud slows down units, hills increase a units field of view, forests decrease a units field of view, and water is impassable terrain. There's also a dark grey "terrain" to represent squares my scout hasn't seen yet. I've made a small test level wherein I'll run my scout.

![Research2](https://github.com/Louis-DV/Real-Time-Scout-AI/blob/master/Images/Research1.JPG "Research2")

Now onto the scout itself. The most important thing is that the scout will move around by itself. For this I've made a list wherein I push every important square that my scout needs to go look at. These important squares are mostly squares that are adjacent to unseen squares, but also squares like hills and forest, that my agent should check out as well. I then take the closest important square to my agent, and have him move there. However, if there's a hill square in the list, he'll move over there first, because his field of view is bigger on the hill, so he'll be able to map out everything more quickly. This way, my scout will eventually have mapped out the whole level.

However, just mapping out the level isn't good enough, the scout should also be able to give every square a score according to its strategic importance and danger level. For this I used a scale from -10 to 10, -10 being extremely dangerous, 10 being extremely important, and 0 being neutral. 
I don't have an actual game here, so to be able to actually give scores, there's a few "rules" and assumptions I've made. 

1) The scout goes through the level before any fighting happens or any enemies show up, so the scores are mostly for preparations.
2) The friendly side is the left side of the level, and the enemies will come from the right side.
3) The scout can look through trees, but normal units wouldn't be able to look through trees.

I'm saving all the scores in a map, with the index of the node as the key, and the score as the value. Most of the nodes will be given a score as soon as the scout sees them, but hills and forests will need to be visited before the scout can give them a score, as he will need to check what can be seen when he's there, as the field of view changes. The scores are a bit random, as I don't have a game, so I can't give a really accurate score, but I assumed some good and some bad positions.

### good:
-Hills
-Forests adjacent to other, passable terrain
-Friendly side of water or mud
-Passages with 2 squares of water or mud either horizontal or vertical

### bad:
-Squares next to hills
-Forests adjacent only to impassable terrain and other forests
-Enemy side of water or mud
-Cornered by water or mud
-Squares next to a forest

I give every square a score with some simple loops and if statements, checking the adjacent squares and positions of water or mud. for hills and forests, I just check every square in the scouts field of view. With this, every square now has a score, giving the rest of the AI the needed information to properly position it's units.

![Research3](https://github.com/Louis-DV/Real-Time-Scout-AI/blob/master/Images/Research3.JPG "Research3")

One problem I have noticed though, is that the scout stops moving after he has seen everything, and that he doesn't reevaluate squares that he has already given a score. This is not a problem on a static level, which I have limited myself to for this project. but if you have a dynamic level, this will be problem. However, this could be easily fixed by giving every square a timer, which represents how long it's been since the scout has evaluated the square, and then after he's gone through the whole level, you can let him reevaluate the squares that haven't been evaluated for a while.


## Conclusion






