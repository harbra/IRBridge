<!--
This vue.js file contains the header navigation for the IRBridge project.
Props can be passed for the title text, available links, etc.
-->
<template>
<div>
  <nav class="navbar">
    <div class="navbar-brand">
      <span class="navbar-item">
        <router-link :to="menulink" tag="a" class="title">
          {{product}}
        </router-link>
      </span>
      <span class="navbar-burger" v-bind:class="{'is-active': burgerOpen}" v-on:click="burgerOpen=!burgerOpen">
            <span></span>
            <span></span>
            <span></span>
      </span>
    </div>

    <div class="navbar-menu" v-bind:class="{'is-active': burgerOpen}">
      <div class="navbar-end">
        <div class="navbar-item">
          <template v-if="connectedNetwork.isConnected">
            <span class="icon">
              <i v-bind:class="'icon-wifi-' + signalQualityforIcon"></i>
            </span>
            <span>
              {{ connectedNetwork.SSID }}
            </span>
            <span class="icon">
              <i class="icon-lock" v-if="connectedNetwork.encrypted"></i>
              <i class="icon-unlock-alt" v-if="!connectedNetwork.encrypted"></i>
            </span>
          </template>
          <span v-else>
            Wifi disconnected.
          </span>
        </div>

        <hr />

        <div class="navbar-item has-dropdown is-hoverable" v-if="menutitle!=null" v-bind:class="{'is-active': menuOpen}">
          <a class="navbar-link" v-on:click="menuOpen=!menuOpen">
            <router-link :to="menulink" tag="div">
              {{menutitle}}
            </router-link>
          </a>
          <div class="navbar-dropdown">
            <router-link v-for="submenu in submenus" class="navbar-item" :key="submenu.linkto" :to="submenu.linkto">{{submenu.linktext}}</router-link>
          </div>
        </div>

        <div class="navbar-item" v-bind:class="{'is-active': settingsOpen, 'has-dropdown': subsettings!==undefined, 'is-hoverable': subsettings!==undefined}">
          <a v-on:click="settingsOpen=!settingsOpen" v-bind:class="{'navbar-link': subsettings!==undefined}" v-if="settingslink!==undefined">
            <router-link :to="settingslink" tag="span" class="icon">
                  <i class="icon-cog"></i>
            </router-link>
            <router-link :to="settingslink" tag="span" class="is-hidden-desktop">{{settingstitle}}</router-link>
          </a>
          <div class="navbar-dropdown is-right" v-if="subsettings!==undefined">
            <router-link v-for="subsetting in subsettings" class="navbar-item" :key="subsetting.linkto" :to="subsetting.linkto">{{subsetting.linktext}}</router-link>
          </div>
        </div>
      </div>
    </div>
  </nav>
  <section class="hero is-info">
    <div class="hero-body">
      <div class="container">
        <h1 class="title">
              {{title}}
            </h1>
        <h2 class="subtitle">
              {{subtitle}}
            </h2>
      </div>
    </div>
    <div class="hero-foot">
      <div class="container">
        <div class="tabs is-boxed">
          <ul>
            <router-link v-for="sublink in sublinks" :key="sublink.linkto" :to="sublink.linkto" tag="li" active-class="is-active"><a>{{sublink.linktext}}</a></router-link>
          </ul>
        </div>
      </div>
      </div>
    </section>
</div>
</template>

<script>
import Vue from 'vue'
import {HTTP} from '../plugins/axios'
import WebEvents from '../plugins/webevents';

export default {
  name: 'top-nav',
  props: {
    product: {
      type: String,
      default: "IRBridge" },
    title: String,
    subtitle: String,
    sublinks: Array, // format: linkto, linktext
    menutitle: {
      type: String,
      default: "Menu" },
    menulink: {
      type: String,
      default: "/" },
    submenus: {
      type: Array,
      default: () => {
        return [{linkto:'/irmonitor', linktext:'IR Monitor'},{linkto:'/devices', linktext:'Devices'},{linkto:'/bookmarks', linktext:'Bookmarks'}];
      }}, // format: linkto, linktext
    settingstitle: {
      type: String,
      default: "Settings" },
    settingslink: {
      type: String,
      default: "/settings" },
    subsettings: {
      type: Array,
      default: () => {
        return [{linkto:'/settings/network', linktext:'Network'},{linkto:'/settings/backup', linktext:'Backup/Restore'}];
      }} // format: linkto, linktext
  },
  data() {
    return {
      burgerOpen: false,
      menuOpen: false,
      settingsOpen: false,
      connectedNetwork: { isConnected: false,
                          SSID: "",
                          RSSI: -100,
                          encrypted: false}
    }
  },
  computed: {
    // signalQuality is between 0% and 100%. An RSSI of -100 means 0%, an RSSI of -50 means 100%. Everything above -50 is considered good signal quality
    signalQuality: function() {
      return Math.min(Math.max(2*(this.connectedNetwork.RSSI+100), 0), 100);
    },
    signalQualityforIcon: function() {
      return Math.min(Math.max(Math.floor(2*(this.connectedNetwork.RSSI+100)/100*4), 0), 4);
    }
  },
  created: function() {
    if(WebEvents.sourceAvailable()) {
      this.$eventsource.addEventListener('ConnectionCheck', e => {
        try {
          this.connectedNetwork=JSON.parse(e.data);
        } catch (err) {
          this.connectedNetwork={isConnected: false};
        }
      }, false);

      this.$eventsource.addEventListener('error', e => {
        if (e.target.readyState != EventSource.OPEN) {
          this.connectedNetwork={isConnected: false};
        }
      }, false);
    }

      HTTP.get('/api/getwifistatus').then((response) =>
        {
          if(response.status==200 && (response.headers["content-type"].includes("application/json") || response.headers["content-type"].includes("text/json")))
          {
            this.connectedNetwork=response.data;
          } else {
            this.connectedNetwork={isConnected: false};
          }
        }).catch((e) =>
        {
          this.connectedNetwork={isConnected: false};
        })
  }
}
</script>
