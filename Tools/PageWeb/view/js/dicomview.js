// Load in HTML templates
var viewportTemplate; // the viewport template
loadTemplate("templates/viewport.html", function (element) {
  viewportTemplate = element;
});
var studyViewerTemplate; // the study viewer template
loadTemplate("templates/studyViewercopy.html", function (element) {
  studyViewerTemplate = element;
});
// var viewportModel = viewportTemplate;
// var studyViewer = studyViewerTemplate; //.clone();
// var imageViewer = new ImageViewer(studyViewer, viewportModel);
var imageViewer;
var viewportModel;
var studyViewer;
cornerstoneWADOImageLoader.external.cornerstone = cornerstone;
ImageViewer = function (root, viewport) {
  var self = this;

  self.root = root;
  self.stacks = [];
  self.viewports = [];
  self.layout = "1x1";
  self.viewportModel = viewport;

  self.setLayout = function (layout) {
    self.layout = layout;
    // TODO: create viewports
    var ab = self.getRowsCols(),
      a = ab[0],
      b = ab[1],
      numOfViewports = a * b,
      perWidth = 100 / b,
      perHeight = 100 / a;
    self.root.find(".imageViewer").html("");
    var i = 0;
    self.viewports = [];
    while (i < numOfViewports) {
      var elem = self.viewportModel
        .clone()
        .css({
          width: perWidth + "%",
          height: perHeight + "%",
        })
        .appendTo(self.root.find(".imageViewer"));
      elem.find(".viewport").data("index", i).data("waiting", true);

      self.viewports.push(elem);
      i++;
    }
  };

  self.getRowsCols = function () {
    var s = self.layout.split(/x/);
    return [parseInt(s[0]), parseInt(s[1])];
  };

  self.isSingle = function () {
    return self.layout == "1x1";
  };

  self.getElement = function (item) {
    return self.viewports[item].find(".viewport")[0];
  };

  self.forEachElement = function (cb) {
    self.viewports.forEach(function (vp, i) {
      cb.call(self, vp.find(".viewport")[0], vp, i);
    });
  };
};

/**
 * sets a new image object for a given element
 * @param element
 * @param image
 */
function displayImage(element, image, viewport) {
  if (element === undefined) {
    throw "displayImage: parameter element cannot be undefined";
  }
  if (image === undefined) {
    throw "displayImage: parameter image cannot be undefined";
  }

  var enabledElement = cornerstone.getEnabledElement(element);

  enabledElement.image = image;

  if (enabledElement.viewport === undefined) {
    enabledElement.viewport = cornerstone.internal.getDefaultViewport(enabledElement.canvas, image);
  }

  // merge viewport
  if (viewport) {
    for (var attrname in viewport) {
      if (viewport[attrname] !== null) {
        enabledElement.viewport[attrname] = viewport[attrname];
      }
    }
  }

  var now = new Date();
  var frameRate;
  if (enabledElement.lastImageTimeStamp !== undefined) {
    var timeSinceLastImage = now.getTime() - enabledElement.lastImageTimeStamp;
    frameRate = (1000 / timeSinceLastImage).toFixed();
  } else {
  }
  enabledElement.lastImageTimeStamp = now.getTime();

  var newImageEventData = {
    viewport: enabledElement.viewport,
    element: enabledElement.element,
    image: enabledElement.image,
    enabledElement: enabledElement,
    frameRate: frameRate
  };

  $(enabledElement.element).trigger("CornerstoneNewImage", newImageEventData);

  cornerstone.updateImage(element);
}

function showStduyImage(studystudyuid) {
  if (studystudyuid.length < 1) {
    $("image").text("no image get from server");
    return -1;
  }
  // $(image).style.height = "100%";
  //eg.: http://127.0.0.1:8080/WADO?studyuid=1.3.6.1.4.1.25403.52237031786.3872.20100510032220.1&type=json
  var host            = window.location.host;
  var server_json_url = "http://" + host + "/";
  var server_wado_url = "wadouri:http://" + host + "/WADO?studyuid=";
  server_json_url     = server_json_url + "WADO?studyuid=" + studystudyuid + "&type=json";
  $.getJSON(server_json_url, function (data) {
    // document.getElementById("image").style.height = parseInt(document.body.clientHeight) - 2 + 'px';
    if (typeof viewportModel === "undefined") {
      viewportModel = viewportTemplate;
    }
    if (typeof studyViewer === "undefined") {
      studyViewer = studyViewerTemplate; //.clone(); remove
    }
    // Make the viewer visible
    // studyViewer.removeClass('hidden');remove
    // Add section to the tab content
    //studyViewer.appendTo('#image');
    $(studyViewer).appendTo("#image");
    // var imageViewer = new ImageViewer(studyViewer, viewportModel);
    if (typeof imageViewer === "undefined") {
      imageViewer = new ImageViewer(studyViewer, viewportModel);
    }
    imageViewer.setLayout("2x2"); // default layout
    function initViewports() {
      imageViewer.forEachElement(function (el) {
        cornerstone.enable(el);
        $(el).droppable({
          drop: function (evt, ui) {
            var fromStack = $(ui.draggable.context).data("stack"),
              toItem = $(this).data("index");
            useItemStack(toItem, fromStack);
          },
        });
      });
    }
    // setup the tool buttons
    setupButtons(studyViewer);
    // layout choose
    $(studyViewer)
      .find(".choose-layout a")
      .click(function () {
        var previousUsed = [];
        imageViewer.forEachElement(function (el, vp, i) {
          if (!isNaN($(el).data("useStack"))) {
            previousUsed.push($(el).data("useStack"));
          }
        });
        var type = $(this).text();
        imageViewer.setLayout(type);
        initViewports();
        resizeStudyViewer();
        if (previousUsed.length > 0) {
          previousUsed = previousUsed.slice(0, imageViewer.viewports.length);
          var item = 0;
          previousUsed.forEach(function (v) {
            useItemStack(item++, v);
          });
        }
      });
    var seriesIndex = 0;
    // Create a stack object for each series
    data.seriesList.forEach(function (series) {
      var stack = {
        seriesDescription: series.seriesDescription,
        stackId: series.seriesNumber,
        imageIds: [],
        seriesIndex: seriesIndex,
        currentImageIdIndex: 0,
        seriesUid: series.seriesUid,
        frameRate: series.frameRate,
      };
      // Populate imageIds array with the imageIds from each series
      // For series with frame information, get the image url's by requesting each frame
      if (series.numberOfFrames !== undefined) {
        var numberOfFrames = series.numberOfFrames;
        for (var i = 0; i < numberOfFrames; i++) {
          var imageId = series.instanceList[0].imageId + "?frame=" + i;
          if (imageId.substr(0, 4) !== "http") {
            imageId = server_wado_url + imageId;
          }
          stack.imageIds.push(imageId);
          console.log(imageId);
        }
        // Otherwise, get each instance url
      } else {
        series.instanceList.forEach(function (image) {
          var imageId = image.imageId;

          if (image.imageId.substr(0, 4) !== "http") {
            imageId = server_wado_url + data.studyuid + "&studyDate=" + data.studyDate + "&seriesuid=" + series.seriesUid + "&sopinstanceuid=" + image.imageId;
          }
          stack.imageIds.push(imageId);
          //console.log(imageId);
        });
      }
      // Move to next series
      seriesIndex++;
      // Add the series stack to the stacks array
      imageViewer.stacks.push(stack);
    });

    // Resize the parent div of the viewport to fit the screen
    var imageViewerElement = $(studyViewer).find(".imageViewer")[0];
    var viewportWrapper = $(imageViewerElement).find(".viewportWrapper")[0];
    var parentDiv = $(studyViewer).find(".viewer")[0];
    var studyRow = $(studyViewer).find(".studyRow")[0];
    var width = $(studyRow).width();

    // Get the viewport elements
    var element = $(studyViewer).find(".viewport")[0];
    // Image enable the dicomImage element
    initViewports();
    //cornerstone.enable(element);
    // Get series list from the series thumbnails (?)
    var seriesList = $(studyViewer).find(".thumbnails")[0];
    imageViewer.stacks.forEach(function (stack, stackIndex) {
      // Create series thumbnail item
      var seriesEntry =
        '<a class="list-group-item" + ' +
        'oncontextmenu="return false"' +
        'unselectable="on"' +
        'onselectstart="return false;"' +
        'onmousedown="return false;">' +
        '<div class="csthumbnail"' +
        'oncontextmenu="return false"' +
        'unselectable="on"' +
        'onselectstart="return false;"' +
        'onmousedown="return false;"></div>' +
        "<div class='text-center small'>" +
        stack.seriesDescription +
        "</div></a>";
      // Add to series list
      var seriesElement = $(seriesEntry).appendTo(seriesList);
      // Find thumbnail
      var thumbnail = $(seriesElement).find("div")[0];
      // Enable cornerstone on the thumbnail
      cornerstone.enable(thumbnail);
      // Have cornerstone load the thumbnail image
      //cornerstone.loadAndCacheImage(imageViewer.stacks[stack.seriesIndex].imageIds[0]).then(function(image) {
      cornerstone
        .loadImage(imageViewer.stacks[stack.seriesIndex].imageIds[0])
        .then(
          function (image) {
            //cornerstone.loadAndCacheImage(ThumbnailID).then(function(image) {
            // Make the first thumbnail active
            if (stack.seriesIndex === 0) {
              $(seriesElement).addClass("active");
            }
            // Display the image
            //console.log(image);
            cornerstone.displayImage(thumbnail, image);
            $(seriesElement).draggable({ helper: "clone" });
          },
          function (err) {
            console.log("loadAndCacheImage error:" + err);
            alert(err);
          }
        );
      // Handle thumbnail click
      $(seriesElement)
        .on("click touchstart", function () {
          useItemStack(0, stackIndex);
        })
        .data("stack", stackIndex);
    });

    function useItemStack(item, stack) {
      var imageId = imageViewer.stacks[stack].imageIds[0],
        element = imageViewer.getElement(item);
      if ($(element).data("waiting")) {
        imageViewer.viewports[item].find(".overlay-text").remove();
        $(element).data("waiting", false);
      }
      $(element).data("useStack", stack);
      displayThumbnail(
        seriesList,
        $(seriesList).find(".list-group-item")[stack],
        element,
        imageViewer.stacks[stack],
        function (el, stack) {
          if (!$(el).data("setup")) {
            setupViewport(el, stack, this);
            setupViewportOverlays(el, data);
            $(el).data("setup", true);
          }
        }
      );
      //cornerstone.loadAndCacheImage(imageId).then(function (image) {
      cornerstone.loadImage(imageId).then(function (image) {
        setupViewport(element, imageViewer.stacks[stack], image);
        setupViewportOverlays(element, data);
      });
    }
    // Resize study viewer
    function resizeStudyViewer() {
      var studyRow = $(studyViewer).find(".studyContainer")[0];
      var height = $(studyRow).height();
      var width = $(studyRow).width();
      //console.log($(studyRow).innerWidth(), $(studyRow).outerWidth(), $(studyRow).width());
      $(seriesList).height("100%");
      $(parentDiv).width(
        width - $(studyViewer).find(".thumbnailSelector:eq(0)").width()
      );
      $(parentDiv).css({ height: "100%" });
      $(imageViewerElement).css({
        height:
          $(parentDiv).height() -
          $(parentDiv).find(".text-center:eq(0)").height(),
      });
      imageViewer.forEachElement(function (el, vp) {
        cornerstone.resize(el, true);
        if ($(el).data("waiting")) {
          var ol = vp.find(".overlay-text");
          if (ol.length < 1) {
            ol = $(
              '<div class="overlay overlay-text">Please drag a stack onto here to view images.</div>'
            ).appendTo(vp);
          }
          var ow = vp.width() / 2,
            oh = vp.height() / 2;
          ol.css({ top: oh, left: ow - ol.width() / 2 });
        }
      });
    }
    // Call resize viewer on window resize
    $(window).resize(function () {
      resizeStudyViewer();
    });
    resizeStudyViewer();
    if (imageViewer.isSingle()) useItemStack(0, 0);
  }).fail(function () {
    console.log("error");
  });
}

//http://127.0.0.1/view/testview.html?1.2.840.113619.2.55.3.604688119.868.1249343483.504
var url = location.search;
var studystudyuid = url.substring(1, url.length);
showStduyImage(studystudyuid);

// Resize main
function resizeMain() {
  var height = $(window).height();
  $("#main").height(height - 50);
  // $('#image').height(height - 50 - 42);
  $("#image").height(height - 2);
}

// Call resize main on window resize
$(window).resize(function () {
  resizeMain();
});
resizeMain();

// Prevent scrolling on iOS
document.body.addEventListener("touchmove", function (e) {
  e.preventDefault();
});
