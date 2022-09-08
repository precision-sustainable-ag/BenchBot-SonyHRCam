## Folders and Files

> ### **GUI**
> The GUI folder contains everything that you need to run this system's user interface. Download this folder to your computer using a downloader like [DownGit](https://minhaskamal.github.io/DownGit/#/home) if you don't want to clone the whole repository.

> ### **Pi Files**
> This folder contains the files that need to be added to the Pi When its sole function is to receive the ultrasonic sensors’ readings and send those to the main host. 
> If the Pi is also being used as the main host then the GUI folder will also be added to the PI.

> ### **.env**
> This file lives localy in your computer and it contains variables whose values are specific to your setup.
> Find below detailed explanations of some of the variables that you find in this file.
>> 
>> **WHEEL_MOTORS:** Which wheel motor you connect to which driver on the Machine Motion (MM) controller matters. If you followed the instructions on how to set up your BenchBot on the [Hardware Section](https://precision-sustainable-ag.atlassian.net/l/cp/JhmoMC2C) on our Confluence page, each wheel motor should be connected to the driver that will make the BenchBot work as intended. If you notice that the BenchBot is not correcting its trajectory and it is drifting away this could be due to the drivers being switched. You have two options here, you can change the drivers to which each motor is connected to on the MM or, you can change the WHEEL_MOTORS values from ‘3,2’ to ‘2,3’ on the .env file.
>> 
>> **DIRECTIONS:** If the BenchBot is moving in the opposite direction that you need it to move, simply change the DIRECTIONS values from ‘negative, positive’ to ‘positive, negative’. 

> ### **environment.yml**
> This file file creates the environment for the UI to run. 
> On the command line run 
>> conda env create -f environment.yml
>>>conda activate bb_env