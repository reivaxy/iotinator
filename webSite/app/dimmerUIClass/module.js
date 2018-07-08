

console.log("DIMMER LOADED");

(function toto() {
  XIOT.loadJS("dimmerUIClass/slider/bootstrap-slider.min.js");
  XIOT.loadCSS("dimmerUIClass/slider/bootstrap-slider.min.css");
})();


var dimmerUIClass = {
  
  // TODO : Not used yet. When modules are loaded dynamicall, this will be called by loader
  // init: function() {
  //   XIOT.loadJS("dimmerUIClass/slider/bootstrap-slider.min.js");  
  //   XIOT.loadCSS("dimmerUIClass/slider/bootstrap-slider.min.css");  
  // },
  
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
      '<div class="dimmerUIClassContent">' +
        '<input id="dimmerUIClassSlider" data-slider-id="dimmerUIClassSliderSlider" type="text" data-slider-min="0" ' +
            'data-slider-max="100" data-slider-step="1" data-slider-value="<%- level %>" data-slider-tooltip="hide"/>' +
      '</div>'
      
    ),
    
    initialize: function () {
      this.listenTo(this.model, 'change', this.render);
    },
    events: {
      "mouseup #dimmerUIClassSliderSlider .slider-handle" : "level", 
      "touchend #dimmerUIClassSliderSlider .slider-handle" : "level" 
    },
    
    level: function(e) {
      let value = $('#dimmerUIClassSlider')[0].value;
      console.log("Slider: ", value);
      // Rebuilding the DOM is not only useless here, it sucks
      // this.model.set('level', value);
      this.model.attributes.level = value;
      this.xiotSync(this.model);
    },
    render: function () {
      let slider = document.getElementById('dimmerUIClassSlider');
      // first time the dom is built, need to add the slider
      if(slider == null) {
        this.$el.html(this.template(this.model.toJSON()));
        setTimeout(function () {
          $('#dimmerUIClassSlider').slider({
            formatter: function (value) {
              return value;
            }
          });
          return this;
        }, 100);
      } else {
        // If slider already exists, just update its value
        $('#dimmerUIClassSlider').slider({value: this.model.get('level')});
      }
    }
  })

  
};