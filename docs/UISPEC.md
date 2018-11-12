UI Specification
-------------------------

```diff
+ WARNING: The user must disable auto-lock in the device settings
```


## Terminology:

**Left click** - clicking and releasing left button

**Right click** - clicking and releasing right button

**Double click** - clicking and releasing both left and right 

**Left hold** - clicking and holding left button

**Right hold** - clicking and holding right button

**Double hold** - clicking and holding both left and right 

## UI pages:
### A. 'Welcome' page

#### Layout:
Screen 1. (default)
```
LINE1:
LINE2:                      Cosmos TEST!
LINE3: [tendermint icon]    Validator       [down icon]
```

*"TEST!" is only shown when the application has been compiled in TEST mode.*

Screen 2.
```
LINE1:
LINE2:  [up icon]             About         [down icon]
LINE3:
```

Screen 3.
```
LINE1:
LINE2: [up icon] [quit icon] Quit app
LINE3:
```
#### Interface:
##### Left click
Moves to the previous screen.
##### Right click
Moves to the next screen.
##### Left and Right clicked together on the Screen #3
Exits the app.

### B. 'Initialization' page

#### Layout:
Screen 1. (default)
```
LINE1:  [X]       Init Validation       [OK]  
LINE2:            Height: {height}
LINE3:            Round:  {round}
```
 - Show in the screen the values and ask for confirmation.
 - While in this mode, any signature requests should be immediately rejected.
 - If the user confirms the values, the ledger will initialize the values and move back to signing mode
 - If the user reject, the ledger will keep values as not-initialized and move back to signing mode 
 
### C. 'Validation' page

#### Layout:
Screen 1. (default)
```
LINE1:            Validating  
LINE2:            Public Key (scrolling)
LINE3:            Height: {height}
```
- Buttons are disabled.
- Screen will go back to main menu after 1 minute. This counter will reset every time there is a signature.
