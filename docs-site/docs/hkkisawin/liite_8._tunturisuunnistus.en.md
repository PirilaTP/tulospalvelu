# Appendix 8. Special features of fell orienteering

## Appendix 8. Special features of fell orienteering

Fell orienteering differs from other competitions in
that in most classes competitors orienteer in pairs and both
members of the pair must punch at all controls.

In the basic competition properties, the sport is defined as
orienteering and the specification as pair orienteering. The program then expects
both cards to be read for all those who have been given two Emit codes.
The results service operator must ensure that no codes are missing, because this is determined
separately for each participant and not by class.

It is advisable to write the name of one member of the pair
in the first name field and the other in the last name field. Otherwise the only difference is in the punch check.

If online timing is used, you should give
the parameter UUSINAIKA with a sufficiently large upper limit so that the time of the latter
participant remains in effect even when the members of a pair for some reason cross
the finish line at clearly different times.

In the punch check the program does not judge the performance
as accepted from the first card when two Emit codes have been given. Likewise,
punch-based time calculation takes place only when both cards have been read,
and the worse time is then selected regardless of the readout order. This
function applies only if no times have been recorded before the punch check.
