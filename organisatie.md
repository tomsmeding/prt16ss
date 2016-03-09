# Functiemodel applicatie.

## Algemene declaraties en classes

In de classes hieronder is slechts de public interface weergegeven.

```cpp
template <typename T>
class Maybe{
public:
    Maybe(T value); //constructs a Just(v)
    
    static Maybe<T> Nothing(); //makes a Nothing() for you

    T fromJust() const; //gets the Just-value from the container, assuming it isn't Nothing
    T fromMaybe(T &def) const; //gets the Just-value if there is one, or a copy of `def` if it's Nothing

    bool isJust() const; //query whether this is a Just-value
    bool isNothing() const; //query whether this is Nothing (==!isJust())
};
```

```cpp
template <typename T,typename U>
class Either{
public:
    static Either<T,U> Left(T value); //makes a Left-value for you
    static Either<T,U> Right(U value); //makes a Right-value for you
      
    T fromLeft(void) const; //gets the Left-value, assuming it is one
    U fromRight(void) const; //gets the Right-value, assuming it is one

    bool isLeft(void) const; //query whether this is a Left-value
    bool isRight(void) const; //query whether this is a Right-value
};
```

```cpp
template <typename T>             //to be used as, for example, ErrorOr<double>: either an error (Left(string)) or
using ErrorOr = Either<string,T>; //a Right(double) -- remember as: Right is the right value, Left is the wrong value
```

```cpp
using Errtype = Maybe<string>; //if there can be an error or nothing
```

```cpp
class CellAddress{
public:
    unsigned int row,column; //left-top is (0,0)!
    // . . . .
    // . . . .
    // . # . .  <- that # is at (1,2) in documentation, and on row 2, column 1, and has representation "B3".
    // . . . .

    static Maybe<CellAddress> fromRepresentation(string repr); //converts something like "A1" to a CellAddress
    
    string toRepresentation(); //returns a string representation of this
};
```


## Interface modules

We hebben een MVC-organisatie, per de opdracht.

### Model

```cpp
class Spreadsheet{
public:
    Errtype saveToDisk(string fname); //asks the model to save itself to the specified file
    ErrorOr<string> loadFromDisk(string fname); //asks the model to load itself from the specified file
    
    Maybe<string> getCellDisplayString(CellAddress addr); //gets string for that cell for display in the sheet;
                                                          //(Nothing if out of bounds)
    Maybe<string> getCellEditString(CellAddress addr); //gets string containing the raw cell data (for editing);
                                                       //(Nothing if out of bounds)
    
    using ChangedList = vector<CellAddress>;
    ChangedList changeCellValue(CellAddress addr,string repr);
      //changes the raw cell data of a cell, returns list of cells changed in sheet (includes
      //edited cell)
  
    Errtype ensureSheetSize(int width,int height); //ensures that the sheet is at least the given size;
                                                   //useful for safe querying
};
```

### View

```cpp
class SheetView{
public:
    void redrawCell(CellAddress addr); //redraws the cell at that address (probably a changed cell);
                                       //method can be implemented to do nothing if outside screen
    void redraw(); //redraws entire (visible) screen

    void setCursorPosition(CellAddress addr); //moves the cursor to that position
    CellAddress getCursorPosition(); //queries the current cursor position
    
    Maybe<string> getStringWithEditWindowOverCell(CellAddress loc,string defval);
      //places an edit window (pop-up?) over the specified cell with the specified default value,
      //and returns the entered value. (Nothing if escape pressed)

    Maybe<string> askStringOfUser(string prompt,string default); //asks a string of the user in the status bar, with
                                                                 //specified prompt and default (pre-filled) value
    void displayStatusString(string s); //displays s in the status bar
};
```

### Controller

```cpp
class SheetController{
public:
    SheetController(); //normal initialisation
    SheetController(string filename); //initialises the controller with a filename given on the command line

    ErrorOr<string> cellString(CellAddress addr); //queries the Model for the display cell string; called by
                                                  //view on redrawing. Should also ensureSheetSize on the Model
};
```

## Behaviour

Hieronder beschreven: zaken/functionaliteit die niet duidelijk zijn uit de comments in de interfaces.

- Controller:
  - Heeft een Model en een View als membervariabelen.
  - Opgestart bij starten programma, eventueel met een bestandsnaam op de command line; in dat geval moet de controller handelen alsof er meteen door de user gevraagd was om dat bestand te laden.
  - Onderhoudt een runloop waarin keyboard input wordt afgehandeld.
  - Moet aan het begin de view redrawen.
  - Pijltjestoetsen verplaatsen de cursor in de view.
  - `Enter` bewerkt de huidige cell met behulp van een edit window.
  - `Backspace` bewerkt de huidige cell naar "".
  - `q` sluit het programma af (eventueel vragen om op te slaan indien van toepassing?).
  - Opslaan (`s`) en laden (`l`) gebruiken de huidige bestandsnaam als die er is; anders wordt die gevraagd met `View::askStringOfUser`.
- Model:
  - Verricht alleen acties "op aanvraag"; vraagt zelf geen input van de gebruiker.
  - `changeCellValue` parset zelf de gegeven string en past die toe op de data. **Bij ongeldige data wordt geen error teruggegeven, maar wordt er een zekere `ERR`-waarde in de cel gezet.** Deze waarde werkt (natuurlijk) door in cellen die ervan afhankelijk zijn.
- View:
  - Als een veranderde cursorpositie ervoor zorgt dat er gescrolld moet worden, moet de view dit transparant doen; dwz. de rest van de code "moet er niets van merken". Wat natuurlijk wel moet gebeuren is het opvragen van `cellString`'s van de Controller voor de nieuw in beeld gebrachte cellen.
  - Initialiseert ncurses in de constructor, en sluit ncurses weer netjes af in de destructor.
