# PityRoll

Bad-luck protection, soft pity and drop guarantees for Unreal Engine 5.8.

A drop chance of five percent means somebody goes eighty tries without it, and that player does not
think *variance* - that player thinks the game is broken. PityRoll puts a floor under bad luck. It
counts consecutive misses **per entry**, so picking up a common cannot reset the counter that was
building towards the rare one.

* Soft pity: the chance climbs with every miss, from an attempt you choose
* Hard pity: the guarantee lands on **exactly** the attempt you named, not one early and not one late
* The effective chance is a pure function, so the number your UI shows is the number the roll used
* A natural success inside the pity window consumes the counter exactly once
* The random value is handed in, so a server, a client prediction and a replay agree
* Every rule is a pure function the component and the tests both call

Documentation: https://wiki.teufel-engineering.com/en/PityRoll/documentation
Support: teufelsilvan@gmail.com

Unreal Engine 5.8 - Win64 - one runtime C++ module - no third-party code - full source included.
