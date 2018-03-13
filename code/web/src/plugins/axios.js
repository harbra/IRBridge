/*jshint esversion: 6 */

import axios from 'axios';

export const HTTP = axios.create({
  //baseURL: `http://192.168.1.148`, // for debugging: Setting baseURL will allow to use the local version of the website while calling the API on the ESP. This needs, e.g. Chrome to be run with the argument --disable-web-security
  timeout: 5000
});
