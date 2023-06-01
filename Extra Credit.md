## Extra Credit 

The extra credit functionality is marked clearly within the code and should be able to be commented out if the game without any additional functionality is to be played. In order to quickly find where in our code the extra credit functionality was implemented, a simple ctrl+f "/* Extra credit ends here */" should be able to quickly bring you where they are. 

### Our additions

#### A range limit on vision

We set a range limit on vision of a diameter of five spots. To clarify, the players will still be able to remember the spots that they have already seen, but they cannot see occupants further than five spots away from them anymore.

#### Stealing gold

We allow players to steal gold from other players by stepping onto them. We steal 20 nuggets from the other player if they have at least 20 nuggets; otherwise we steal all of their nuggets.








