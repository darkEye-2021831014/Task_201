# Snake Game

A Basic Snake Game, using sdl2 and c++

## Table of Contents

- [Project Title](#project-title)
- [Description](#description)
- [Features](#features)
- [Requirements](#requirements)
- [Note](#note)

## Description

This Is A Game Project For My University Course SWE-150
(1st year 2nd semester)

## Requirements

sdl2, sdl2_ttf library and c++ 11 or higher and macOS(As i don't know how the game will act in any other operating system)

## Features

You can use up, down, left and right key for changing snake direction(You can also use w,s,a,d for the same type of movement).
k or space will respectively pause and resume the game.
q button can be used to quit the game.

## Note

snake has an initial velocity of 6 and it will increase based on a pseudo-random value x (i.e. 10<=x<=20) up to 15.

The psedo-random value x is a certain value that will be calculated at the start of the game and will remain the same until the game is over.
the velocity of the snake will increase after exactly x scores.

After The Snake Consumes Exactly 7 Food There Will Be A Bonus Food Along With The Regular Food That Will last for 4 seconds and Will Give 10 Points after eating it.
