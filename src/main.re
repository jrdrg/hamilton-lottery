/*
    make GET request to hamilton site
    look for form, get action and store it
    solve captcha via api somehow
    get all form fields and POST back
 */
open BsPuppeteer;

open Js.Promise;

module Dotenv = {
  [@bs.module "dotenv"] external config : unit => unit = "";
  [@bs.val] [@bs.scope "process"] external env : Js.Dict.t(string) = "";
  let get = Js.Dict.get(env);
};

Dotenv.config();

let orDefault = (default, str) =>
  switch str {
  | Some(value) => value
  | None => default
  };

let openLotteryPage = Page.goto("http://www.luckyseat.com/hamilton-ny/", ());

let resolveWith = (a, ()) => resolve(a);

/*
 *  Given a function that takes 'a and returns a promise('b), and a default
 *  value of 'b, returns the result of the function if value is Some or a
 *  promise that resolves to the default if it's None
 */
let maybeResolve = (func, default, value) =>
  switch (Js.Nullable.to_opt(value)) {
  | Some(v) => func(v)
  | None => resolve(default)
  };

let logString = (response) =>
  response
  |> Response.text
  |> then_(
       (json) => {
         Js.log(json);
         resolve(response)
       }
     );

/*
 *  Get a reference to the form
 */
let getForm = (page) => page |> Page.query("form");

let typeInInput = (text, element) =>
  element
  |> ElementHandle.type_(text, ())
  |> then_(resolveWith(element))
  |> then_(ElementHandle.dispose);

let populateField = (field, text, form) =>
  form
  |> ElementHandle.query(field)
  |> then_(maybeResolve(typeInInput(text), ()))
  |> then_(resolveWith(form));

let clickElement = (field, form) =>
  form
  |> ElementHandle.query(field)
  |> then_(
       maybeResolve(
         (el) =>
           el |> ElementHandle.click() |> then_(resolveWith(el)) |> then_(ElementHandle.dispose),
         ()
       )
     )
  |> then_(resolveWith(form));

/*
 *  Populate each of the form field values (this was the old form fields)
 */
let oldPopulateFormValues = (form) =>
  form
  |> populateField("#firstname", Dotenv.get("firstname") |> orDefault(""))
  |> then_(populateField("#lastname", Dotenv.get("lastname") |> orDefault("")))
  |> then_(populateField("#email", Dotenv.get("email") |> orDefault("")))
  |> then_(populateField("#zipcode", Dotenv.get("zipcode") |> orDefault("")))
  |> then_(populateField("#age", Dotenv.get("age") |> orDefault("")))
  |> then_(clickElement("label[for=mobile_notification]"))
  |> then_(clickElement("label[for=two_tickets]"))
  |> then_(clickElement("input[type=submit]"))
  |> then_((_) => resolve());

/*
 *  Populate each of the form field values (new form, with no captcha)
 */
let populateFormValues = (form) =>
  form
  |> populateField("#Q3_1", Dotenv.get("firstname") |> orDefault(""))
  |> then_(populateField("#Q4_1", Dotenv.get("lastname") |> orDefault("")))
  |> then_(populateField("#Q5_1", Dotenv.get("email") |> orDefault("")))
  |> then_(populateField("#Q6_1", Dotenv.get("zipcode") |> orDefault("")))
  |> then_(populateField("#Q11_1", Dotenv.get("age") |> orDefault("")))
  |> then_(clickElement("label[for=Q9_1"))
  |> then_(clickElement("label[for=Q10_2]"))
  |> then_(clickElement("input[type=submit]"))
  |> then_((_) => resolve());

/*
 *  Open the page, get a reference to the form, and populate the fields
 */
let manipulatePage = (page) =>
  page
  |> openLotteryPage
  |> then_(
       (_response) =>
         /* logString(response) |> ignore; */
         getForm(page)
     )
  |> then_(
       (maybeForm) =>
         switch (Js.Nullable.to_opt(maybeForm)) {
         | Some(form) => populateFormValues(form)
         | None =>
           Js.log("-- No form was found --");
           resolve()
         }
     );

let closeBrowser = (browser, ()) => browser |> Browser.close;

let loadPage = (browser) =>
  browser |> Browser.newPage |> then_(manipulatePage) |> then_(closeBrowser(browser));

/* Start everything */
Puppeteer.launch(~options=Launcher.makeLaunchOptions(~headless=false, ()), ())
|> then_((browser) => loadPage(browser));