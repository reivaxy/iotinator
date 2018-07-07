
console.log("DIMMER LOADED");
var dimmerUIClass = {
  // Model for the dimmer module
  Model: Backbone.Model.extend({
    defaults: function () {
      let model = {
        level: 0
      };
      return model;
    }
  }),

  // View for the dimmer module
  View: XIOT.View.extend({
    tagName: "div",
    template: _.template(
      '<div class="progress">' +
        '<div class="progress-bar" role="progressbar" aria-valuenow="<%- level %>" aria-valuemin="0" aria-valuemax="100" style="width: <%- level %>%;">' +
          '<span class="sr-only">Level <%- level %></span>' +
        '</div>' +
      '</div>'
      
    ),
    
    initialize: function () {
      this.listenTo(this.model, 'change', this.render);
    },
    events: {
      "click label.btn": "speed",
      "click button.btn": "oscillation",
    },
    speed: function(e) {
      let label = e.target;
      let value = parseInt(label.getAttribute("value"));
      // We don't want to refresh the DOM to keep default bootstrap button handling
      // this.model.set({speed: value});
      this.model.set('speed', value);
      this.xiotSync(this.model);
    },
    oscillation: function(e) {
      let button = e.target;
      let previous = button.getAttribute("aria-pressed");
      if("true" === previous) {
        this.model.set('osc', "off");
      } else {
        this.model.set('osc', "on");
      }
      this.xiotSync(this.model);
    },
    render: function () {
      this.$el.html(this.template(this.model.toJSON()));
      return this;
    }
  })

  
};