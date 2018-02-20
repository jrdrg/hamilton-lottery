/*
    make GET request to hamilton site
    look for form, get action and store it
    solve captcha via api somehow
    get all form fields and POST back
 */
open BsPuppeteer;

let openLotteryPage = Page.goto("http://www.luckyseat.com/hamilton-ny/", ());

let wrapInPromise = (a, ()) => Js.Promise.resolve(a);

/*
 *  Given a function that takes 'a and returns a promise('b), and a default
 *  value of 'b, returns the result of the function if value is Some or a
 *  promise that resolves to the default if it's None
 */
let wrapInOptionalPromise = (func, default, value) =>
  switch (Js.Nullable.to_opt(value)) {
  | Some(v) => func(v)
  | None => Js.Promise.resolve(default)
  };

let logString = (response) =>
  response
  |> Response.text
  |> Js.Promise.then_(
       (json) => {
         Js.log(json);
         Js.Promise.resolve(response)
       }
     );

/*
 *  Get a reference to the form
 */
let getForm = (page) => page |> Page.query("#clickdimensionsForm");

let typeInInput = (text, element) =>
  element
  |> ElementHandle.type_(text, ())
  |> Js.Promise.then_(wrapInPromise(element))
  |> Js.Promise.then_(ElementHandle.dispose);

let populateField = (field, text, form) =>
  form
  |> ElementHandle.query(field)
  |> Js.Promise.then_(wrapInOptionalPromise(typeInInput(text), ()))
  |> Js.Promise.then_(wrapInPromise(form));

/*
 *  Populate each of the form field values
 */
let populateFormValues = (form) =>
  form
  |> populateField("#performance", "2018-02-21")
  |> Js.Promise.then_(populateField("#firstName", "test"))
  |> Js.Promise.then_(populateField("#lastName", "test"))
  |> Js.Promise.then_(populateField("#email", "test"))
  |> Js.Promise.then_((_) => Js.Promise.resolve());

/*
 *  Open the page, get a reference to the form, and populate the fields
 */
let manipulatePage = (page) =>
  page
  |> openLotteryPage
  |> Js.Promise.then_(
       (response) => {
         logString(response) |> ignore;
         let maybeForm = getForm(page);
         maybeForm
       }
     )
  |> Js.Promise.then_(wrapInOptionalPromise(populateFormValues, ()));

let closeBrowser = (browser, ()) => browser |> Browser.close;

let loadPage = (browser) =>
  browser
  |> Browser.newPage
  |> Js.Promise.then_(manipulatePage)
  |> Js.Promise.then_(closeBrowser(browser));

/* Start everything */
Puppeteer.launch(~options=Launcher.makeLaunchOptions(~headless=false, ()), ())
|> Js.Promise.then_((browser) => loadPage(browser));