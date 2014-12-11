---
title: WIP--Diagonal Dominance Peturbation in the JMJ Approach--WIP
subtitle: Havok Technical Report
author: Jad Nohra
---

# Introduction
Within the context of rigid body simulation, we consider the well know[^1] $$A=JM^{-1}J^T$$ matrix, which appears on the left hand side of an MLCP problem. If an iterative solver is chosen to solve the MLCP problem, guaranteeing convergence and speeding it up are two main concerns. For a PGS solver, convergence is only guaranteed if $A$ is diagonally dominant[^0]. Also, convergence is sped up as the diagonal dominance increases[^0]. When $A$ is not dominant[^2], perturbed versions $A'$ of it which are as close to being so can be easily constructed, using a single positive number $d$ as follows: $$A'=A+dI.$$
It is easy to see that when $d$ is larger than a 'domination threshold' $d'$, $A'$ is dominant.

# Scratch
Therefore, for both concerns, when $A$ is not already diagonally dominant, it can be advantageous to solve an alternative matrix $A'$ in which the diagonal 

Independetely of the chosen MLCP solver type, there is almost always a benefit from replacing such a matrix ($A$) with a diagonally perturbed version of it ($A'$). The pertrubation uses a single real number $d$ in the following manner: $$A'=A+dI.$$ 

Each such matrix $A$ can be perturbed into another matrix $A'$ by adding the identity matrix scaled by some positive scalar $d$ $$A'=A+dI.$$ Numerical overflow aside, it is clear that $A'$ becomes (diagonally) dominant as $d$ surpasses some threshold value $d'$. There are advantages for such a perturbation for both iterative and direct MLCP solvers. For iterative solvers such as PSOR or PGS, convergence is accelerated as $d$ increases, and theoretically guaranteed when $A'$ is dominant, viz. for $d>d'$. For direct solvers that can solve P matrices (e.g Lemke), diagonally dominant matrices are a subset P, hence solvable. Additionally, in practice, even for small $d<d'$, some unsolvable matrices become solvable. 

# Scratch
Each such matrix $A$ is obtained from the systems inverse mass matrix $M^{-1}$ and Jacobian matrix $J$ using the equation $$A = JM^{-1}J^T.$$ 
$M^{-1}$ is PD and has the form $$M^{-1}=()$$. $J$ contains one row for every one-dimensional constraint, such that the bodies directly constrained (usually only two) by it are the only ones with non-zero columns. Per example, a two constraint Jacobian in a three body system might look like this: $$J=().$$ 
in it  The Jacobian    These matrices are symmetric positive definite and are computed using . It is beneficial to perturb such matrices by adding a diagonal matrix $dI^2 + \sqrt(2)$ 
Perturbing the diagonal of an MLCP matrix diagonally dom

# Notes
search for 'system matrix' in erleben's thesis, and the 112 reference. also see erleben p106.
Also see my handwritten notes p36,37.

# Appendix
## A. Calculation of the Domination Threshold
Note that the diagonal elements of $A$ are positive because I is PD, so all diag elements being sums of jijT elements are positive, hence d can be a single positive number...

[^0]: TODO
[^1]: Baraff, *Linear-Time Dynamics using Lagrange Multipliers*.
[^2]: By 'dominant' we will always mean diagonally dominant.
